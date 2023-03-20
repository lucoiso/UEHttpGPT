// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTEditor.h"
#include "SHttpGPTChatView.h"
#include <ToolMenus.h>
#include <Widgets/Docking/SDockTab.h>
#include <WorkspaceMenuStructure.h>
#include <WorkspaceMenuStructureModule.h>

static const FName HttpGPTEditorTabName("HttpGPTEditor");

#define LOCTEXT_NAMESPACE "FHttpGPTEditorModule"

void FHttpGPTEditorModule::StartupModule()
{
	const auto RegisterDelegate = FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FHttpGPTEditorModule::RegisterMenus);
	UToolMenus::RegisterStartupCallback(RegisterDelegate);
}

void FHttpGPTEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(HttpGPTEditorTabName);
}

TSharedRef<SDockTab> FHttpGPTEditorModule::OnSpawnTab(const FSpawnTabArgs& SpawnTabArgs) const
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		[
			SNew(SHttpGPTChatView)
		];
}

void FHttpGPTEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	const auto EditorTabSpawnerDelegate = FOnSpawnTab::CreateRaw(this, &FHttpGPTEditorModule::OnSpawnTab);

#if ENGINE_MAJOR_VERSION < 5
	const FName AppStyleName = FEditorStyle::GetStyleSetName();
#else
	const FName AppStyleName = FAppStyle::GetAppStyleSetName();
#endif

	const TSharedPtr<FWorkspaceItem> Menu = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(LOCTEXT("HttpGPTCategory", "HttpGPT"), LOCTEXT("HttpGPTCategoryTooltip", "HttpGPT Plugin Tabs"), FSlateIcon(AppStyleName, "Icons.Package"));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(HttpGPTEditorTabName, EditorTabSpawnerDelegate)
		.SetDisplayName(FText::FromString("HttpGPT Chat"))
		.SetTooltipText(FText::FromString("Open HttpGPT Chat"))
		.SetIcon(FSlateIcon(AppStyleName, "Icons.Plus"))
		.SetGroup(Menu.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FHttpGPTEditorModule, HttpGPTEditor)