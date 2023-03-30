// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Tasks/HttpGPTBaseTask.h"
#include "Management/HttpGPTSettings.h"
#include "HttpGPTInternalFuncs.h"
#include "LogHttpGPT.h"
#include <HttpModule.h>
#include <Async/Async.h>

#if WITH_EDITOR
#include <Editor.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTBaseTask)
#endif

void UHttpGPTBaseTask::Activate()
{
	Super::Activate();

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Activating task"), *FString(__func__), GetUniqueID());

	bIsTaskActive = true;
	APIKey = UHttpGPTSettings::Get()->APIKey;

	if (!CanActivateTask())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Failed to activate task."), *FString(__func__), GetUniqueID());
		RequestFailed.Broadcast();
		SetReadyToDestroy();
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[this]
		{
			SendRequest();
		}
	);

#if WITH_EDITOR
	if (bIsEditorTask)
	{
		SetFlags(RF_Standalone);
	}
	else
	{
		FEditorDelegates::PrePIEEnded.AddUObject(this, &UHttpGPTBaseTask::PrePIEEnded);
	}
#endif
}

void UHttpGPTBaseTask::StopHttpGPTTask()
{
	FScopeLock Lock(&Mutex);

	if (!bIsTaskActive)
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Stopping task"), *FString(__func__), GetUniqueID());

	bIsTaskActive = false;

	if (HttpRequest.IsValid())
	{
		HttpRequest->CancelRequest();
		HttpRequest.Reset();
	}

	SetReadyToDestroy();
}

void UHttpGPTBaseTask::SetReadyToDestroy()
{
	FScopeLock Lock(&Mutex);

	if (bIsReadyToDestroy)
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Setting task as Ready to Destroy"), *FString(__func__), GetUniqueID());

#if WITH_EDITOR
	if (bIsEditorTask)
	{
		ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
		MarkAsGarbage();
#else
		MarkPendingKill();
#endif
	}

	if (FEditorDelegates::PrePIEEnded.IsBoundToObject(this))
	{
		FEditorDelegates::PrePIEEnded.RemoveAll(this);
	}
#endif

	bIsReadyToDestroy = true;
	bIsTaskActive = false;

	Super::SetReadyToDestroy();
}

const FName UHttpGPTBaseTask::GetAPIKey() const
{
	return APIKey;
}

#if WITH_EDITOR
void UHttpGPTBaseTask::PrePIEEnded(bool bIsSimulating)
{
	if (!IsValid(this))
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Trying to finish task due to PIE end"), *FString(__func__), GetUniqueID());

	bEndingPIE = true;
	StopHttpGPTTask();
}
#endif

bool UHttpGPTBaseTask::CanActivateTask() const
{
	return !HttpGPT::Internal::HasEmptyParam(APIKey);
}

void UHttpGPTBaseTask::SendRequest()
{
	FScopeLock Lock(&Mutex);

	InitializeRequest();
	SetRequestContent();
	BindRequestCallbacks();

	if (!HttpRequest.IsValid())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Failed to send request: Request object is invalid"), *FString(__func__), GetUniqueID());
		
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				RequestFailed.Broadcast();
				SetReadyToDestroy();
			}
		);

		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Sending request"), *FString(__func__), GetUniqueID());

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Request sent"), *FString(__func__), GetUniqueID());

		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				RequestSent.Broadcast();
			}
		);
	}
	else
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Failed to initialize the request process"), *FString(__func__), GetUniqueID());
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				RequestFailed.Broadcast();
				SetReadyToDestroy();
			}
		);
	}
}

bool UHttpGPTTaskStatus::IsTaskActive(const UHttpGPTBaseTask* Test)
{
	return IsValid(Test) && Test->bIsTaskActive;
}

bool UHttpGPTTaskStatus::IsTaskReadyToDestroy(const UHttpGPTBaseTask* Test)
{
	return IsValid(Test) && Test->bIsReadyToDestroy;
}

bool UHttpGPTTaskStatus::IsTaskStillValid(const UHttpGPTBaseTask* Test)
{
	bool bOutput = IsValid(Test) && !IsTaskReadyToDestroy(Test);

#if WITH_EDITOR
	bOutput = bOutput && !Test->bEndingPIE;
#endif

	return bOutput;
}
