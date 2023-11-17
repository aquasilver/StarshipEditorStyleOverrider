// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipStyleOverriderModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IEditorStyleModule.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "StarshipOverrideStyle.h"
#include "StarshipOverrideStyleCustomization.h"
#include "StarshipOverrideStyleSettings.h"
#include "StarshipOverrideThemeManager.h"

#define LOCTEXT_NAMESPACE "StarshipEditorStyleOverrider"

class FStarshipStyleOverriderModule : public IEditorStyleModule
{
public:
	FStarshipStyleOverriderModule()
		: bIsInitialized(false)
	{
	}

	void StartupModule() override
	{
		ModulesChangedHandle = FModuleManager::Get().OnModulesChanged().AddRaw(this, &FStarshipStyleOverriderModule::OnModuleChanged);
	}

	void ShutdownModule() override
	{
		if (ModulesChangedHandle.IsValid())
		{
			FModuleManager::Get().OnModulesChanged().Remove(ModulesChangedHandle);
			ModulesChangedHandle.Reset();
			if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
			{
				SettingsModule->UnregisterSettings("Editor", "General", "StarshipOverrider");
			}
			FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			PropertyEditorModule.UnregisterCustomClassLayout(UStarshipOverrideStyleSettings::StaticClass()->GetFName());
			FStarshipOverrideStyle::Shutdown();
		}
		bIsInitialized = false;
	}

	void OnModuleChanged(FName ModuleName, EModuleChangeReason ChangeReason)
	{
		if (ModuleName == "EditorStyle" && ChangeReason == EModuleChangeReason::ModuleLoaded)
		{
			if (!bIsInitialized)
			{
				UStarshipOverrideStyleThemeManager::Get().LoadThemes();
				UStarshipOverrideStyleSettings* Settings = GetMutableDefault<UStarshipOverrideStyleSettings>();
				Settings->Init();

				ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");


				if (SettingsModule != nullptr)
				{
					ISettingsSectionPtr StyleSettingsPtr = SettingsModule->RegisterSettings("Editor", "General", "StarshipOverrider",
						LOCTEXT("StarshipOverrider_UserSettingsName", "StarshipOverrider"),
						LOCTEXT("StarshipOverrider_UserSettingsDescription", "Customize the look of the editor."),
						Settings
					);
				}

				FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
				PropertyEditorModule.RegisterCustomClassLayout(UStarshipOverrideStyleSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FStarshipOverrideStyleCustomization::MakeInstance));

				FStarshipOverrideStyle::Initialize();

				bIsInitialized = true;
			}
			FAppStyle::SetAppStyleSetName(FStarshipOverrideStyle::GetStyleSetName());
		}
	}

	FDelegateHandle ModulesChangedHandle;
	bool bIsInitialized;
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStarshipStyleOverriderModule, StarshipEditorStyleOverrider );
