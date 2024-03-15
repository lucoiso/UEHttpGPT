// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTEditorModule.h"
#include "Chat/SHttpGPTChatShell.h"
#include "ImageGen/SHttpGPTImageGenView.h"
#include <ToolMenus.h>
#include <Widgets/Docking/SDockTab.h>
#include <WorkspaceMenuStructure.h>
#include <WorkspaceMenuStructureModule.h>

static const FName HttpGPTChatTabName("HttpGPTChat");
static const FName HttpGPTImageGeneratorTabName("HttpGPTImageGenerator");

#define LOCTEXT_NAMESPACE "FHttpGPTEditorModule"

void FHttpGPTEditorModule::StartupModule()
{
	const FSimpleDelegate RegisterDelegate = FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FHttpGPTEditorModule::RegisterMenus);
	UToolMenus::RegisterStartupCallback(RegisterDelegate);
}

void FHttpGPTEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(HttpGPTChatTabName);
}

TSharedRef<SDockTab> FHttpGPTEditorModule::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs) const
{
	const FName TabId = *SpawnTabArgs.GetTabId().ToString();

	TSharedPtr<SWidget> OutContent;
	if (TabId.IsEqual(HttpGPTChatTabName))
	{
		OutContent = SNew(SHttpGPTChatShell);
	}
	else if (TabId.IsEqual(HttpGPTImageGeneratorTabName))
	{
		OutContent = SNew(SHttpGPTImageGenView);
	}

	if (OutContent.IsValid())
	{
		return SNew(SDockTab).TabRole(NomadTab)
			[
				OutContent.ToSharedRef()
			];
	}

	return SNew(SDockTab);
}

void FHttpGPTEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	const FOnSpawnTab EditorTabSpawnerDelegate = FOnSpawnTab::CreateRaw(this, &FHttpGPTEditorModule::OnSpawnTab);

#if ENGINE_MAJOR_VERSION < 5
    const FName AppStyleName = FEditorStyle::GetStyleSetName();
#else
	const FName AppStyleName = FAppStyle::GetAppStyleSetName();
#endif

	const TSharedRef<FWorkspaceItem> Menu = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(
		LOCTEXT("HttpGPTCategory", "HttpGPT"), LOCTEXT("HttpGPTCategoryTooltip", "HttpGPT Plugin Tabs"),
		FSlateIcon(AppStyleName, "Icons.Documentation"));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HttpGPTChatTabName, EditorTabSpawnerDelegate).
	                          SetDisplayName(FText::FromString(TEXT("HttpGPT Chat"))).SetTooltipText(FText::FromString(TEXT("Open HttpGPT Chat"))).
	                          SetIcon(FSlateIcon(AppStyleName, "DerivedData.ResourceUsage")).SetGroup(Menu);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HttpGPTImageGeneratorTabName, EditorTabSpawnerDelegate).
	                          SetDisplayName(FText::FromString(TEXT("HttpGPT Image Generator"))).SetTooltipText(
		                          FText::FromString(TEXT("Open HttpGPT Image Generator"))).SetIcon(
		                          FSlateIcon(AppStyleName, "LevelEditor.Tabs.Viewports")).SetGroup(Menu);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHttpGPTEditorModule, HttpGPTEditor)
