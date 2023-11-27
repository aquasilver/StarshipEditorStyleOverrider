// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideThemeManager.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/ToolBarStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyle.h"
#include "StarshipOverrideStyleSettings.h"

#define LOCTEXT_NAMESPACE "StarshipEditorStyleOverrider"

const FString UStarshipOverrideStyleThemeManager::ThemesSubDir = TEXT("Slate/StarshipOverridThemes");

/* FSlateGetWidget
 *****************************************************************************/
class FSlateGetWidget : public FSlateStyleSet
{
public:
	FSlateGetWidget(const FName& ParentName)
		: FSlateStyleSet("TempSlate")
	{
		SetParentStyleName(ParentName);
	}

	const FSlateWidgetStyle* GetWidgetStyle(const FName& InDesiredTypeName, const FName& InStyleName) const
	{
		return FSlateStyleSet::GetWidgetStyleInternal(InDesiredTypeName, InStyleName, nullptr, false);
	}

	void LogMissingResource(EStyleMessageSeverity Severity, const FText& Message, const FName& MissingResource) const override
	{
		// Ignore
	}

};

/* FStarshipEditorOverrideStyle
 *****************************************************************************/
UStarshipOverrideStyleThemeManager::UStarshipOverrideStyleThemeManager()
{
}

void UStarshipOverrideStyleThemeManager::LoadThemes()
{
	CalcStyleDefines();
	Themes.Empty();

	TSharedPtr<IPlugin> FoundPlugin = IPluginManager::Get().FindPlugin("StarshipEditorStyleOverrider");
	check(FoundPlugin);

	FString PluginPath = FoundPlugin->GetBaseDir();
	LoadThemesFromDirectory(FPaths::Combine(PluginPath, "Content"));

	check(Themes.Num() == 1);
	CurrentThemeId = Themes[0].Id;
	DefaultThemeId = CurrentThemeId;

	// Project themes
	LoadThemesFromDirectory(GetProjectThemeDir());

	// User specific themes
	LoadThemesFromDirectory(GetUserThemeDir());

	ApplyTheme(CurrentThemeId);
}

void UStarshipOverrideStyleThemeManager::SaveCurrentThemeAs(const FString& InFilename, const FStarshipOverrideStyleRawPtrData& InSaveData)
{
	FStarshipOverrideStyleTheme& CurrentTheme = GetMutableCurrentTheme();
	CurrentTheme.Filename = InFilename;
	FString NewPath = CurrentTheme.Filename;
	TSharedRef<FJsonObject> SaveJsonObject = MakeShared<FJsonObject>();
	{
		FString Output;
		TSharedRef<TJsonWriter<>> WriterRef = TJsonWriterFactory<>::Create(&Output);
		TJsonWriter<>& Writer = WriterRef.Get();

		TSharedRef<FJsonObject> RootJsonObject = MakeShared<FJsonObject>();
		RootJsonObject->SetNumberField("Version", 1);
		RootJsonObject->SetStringField("Id", CurrentTheme.Id.ToString());
		RootJsonObject->SetStringField("DisplayName", CurrentTheme.DisplayName.ToString());

		{
			TSharedRef<FJsonObject> EditedJsonObject = InSaveData.ToJsonObject();
			FStarshipOverrideStyleRawPtrData TempDefaultData;
			TempDefaultData.Create(DefaultEditorStyleData);
			TempDefaultData.UnlinkColors();
			TSharedRef<FJsonObject> DefaultJsonObject = TempDefaultData.ToJsonObject();

			CreateDiffJsonObject(SaveJsonObject.Get(), EditedJsonObject.Get(), DefaultJsonObject.Get());
			
			RootJsonObject->SetObjectField("Styles", SaveJsonObject);
		}

		FJsonSerializer::Serialize(RootJsonObject, Writer, true);

		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*InFilename))
		{
			FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*InFilename, false);
			// create a new path if the filename has been changed. 
			NewPath = GetUserThemeDir() / CurrentTheme.DisplayName.ToString() + TEXT(".json");

			if (!NewPath.Equals(CurrentTheme.Filename))
			{
				// rename the current .json file with the new name. 
				IFileManager::Get().Move(*NewPath, *InFilename);
			}
		}
		FFileHelper::SaveStringToFile(Output, *NewPath);
	}


	//Once the default state is restored, the change data must be reapplied, or the FSlateColor::Unlink state will be included.
	CurrentTheme.ObjectData.CopyData(DefaultEditorStyleData);
	CurrentTheme.ObjectData.SetJsonObject(SaveJsonObject);
	OnThemeUpdated().Broadcast(CurrentThemeId); 
}

void UStarshipOverrideStyleThemeManager::ApplyTheme(const FGuid& InThemeId)
{
	if (InThemeId.IsValid())
	{
		FStarshipOverrideStyleTheme* CurrentTheme = nullptr;
		if (CurrentThemeId != InThemeId)
		{
			if (CurrentThemeId.IsValid())
			{
				CurrentTheme = &GetMutableCurrentTheme();
			}

			FStarshipOverrideStyleTheme* Theme = Themes.FindByKey(InThemeId);
			if (Theme)
			{
				CurrentThemeId = InThemeId;
			}
		}

		CurrentTheme = &GetMutableCurrentTheme();
		LoadThemeStyles(*CurrentTheme);
		OnThemeChanged().Broadcast(CurrentThemeId); 
	}
}

void UStarshipOverrideStyleThemeManager::ApplyDefaultTheme()
{
	ApplyTheme(DefaultThemeId);
}

void UStarshipOverrideStyleThemeManager::RemoveTheme(const FGuid& InThemeId)
{
	// Current Theme cannot currently be removed.  Apply a new theme first
	if (CurrentThemeId != InThemeId)
	{
		Themes.RemoveAll([&InThemeId](const FStarshipOverrideStyleTheme& InTheme) { return InTheme.Id == InThemeId; });
	}
}

FGuid UStarshipOverrideStyleThemeManager::DuplicateActiveTheme()
{
	const FStarshipOverrideStyleTheme& CurrentTheme = GetCurrentTheme();

	FGuid NewThemeGuid = FGuid::NewGuid();
	FStarshipOverrideStyleTheme NewTheme;
	NewTheme.Id = NewThemeGuid;
	NewTheme.DisplayName = FText::Format(LOCTEXT("ThemeDuplicateCopyText", "{0} - Copy"), CurrentTheme.DisplayName);

	Themes.Add(MoveTemp(NewTheme));

	return NewThemeGuid;
}

void UStarshipOverrideStyleThemeManager::ValidateActiveTheme()
{
	ReloadConfig();
	//EnsureValidCurrentTheme();
	ApplyTheme(GetCurrentTheme().Id);
}

void UStarshipOverrideStyleThemeManager::SetCurrentThemeDisplayName(const FText& InNewDisplayName)
{
	GetMutableCurrentTheme().DisplayName = InNewDisplayName;
}

FString UStarshipOverrideStyleThemeManager::GetProjectThemeDir() const
{
	return FPaths::ProjectContentDir() / ThemesSubDir;
}

FString UStarshipOverrideStyleThemeManager::GetUserThemeDir() const
{
	return FPaths::Combine(FPlatformProcess::UserSettingsDir(), *FApp::GetEpicProductIdentifier()) / ThemesSubDir;
}

void UStarshipOverrideStyleThemeManager::LoadThemesFromDirectory(const FString& InDirectory)
{
	TArray<FString> ThemeFiles;
	IFileManager::Get().FindFiles(ThemeFiles, *InDirectory, TEXT(".json"));

	for (const FString& ThemeFile : ThemeFiles)
	{
		bool bValidFile = false;
		FString ThemeData;
		FString ThemeFilename = InDirectory / ThemeFile;
		if (FFileHelper::LoadFileToString(ThemeData, *ThemeFilename))
		{
			FStarshipOverrideStyleTheme Theme;
			if (ReadTheme(ThemeData, Theme))
			{
				if (FStarshipOverrideStyleTheme* ExistingTheme = Themes.FindByKey(Theme.Id))
				{
					// Just update the existing theme.  Themes with the same id can override an existing one.  This behavior mimics config file hierarchies
					ExistingTheme->Filename = MoveTemp(ThemeFilename);
				}
				else
				{
					// Theme not found, add a new one
					Theme.Filename = MoveTemp(ThemeFilename);
					Themes.Add(MoveTemp(Theme));
				}
			}
		}
	}
}

void UStarshipOverrideStyleThemeManager::LoadThemeStyles(FStarshipOverrideStyleTheme& InOutTheme)
{
	FString ThemeData;


	{
		const int32 HashForNotation = (int32)GetTypeHash(InOutTheme.Id);
		const FName WrapperClassName(FString::Printf(TEXT("StarshipOverrideStyle%d"), HashForNotation));

		InOutTheme.ObjectData.Create(DefaultEditorStyleData);
	}
	
	if (FFileHelper::LoadFileToString(ThemeData, *InOutTheme.Filename))
	{
		//Theme.LoadedDefaultColors.Empty();
		TSharedRef<TJsonReader<>> ReaderRef = TJsonReaderFactory<>::Create(ThemeData);
		TJsonReader<>& Reader = ReaderRef.Get();

		TSharedPtr<FJsonObject> ObjectPtr;
		if (FJsonSerializer::Deserialize(Reader, ObjectPtr) && ObjectPtr.IsValid())
		{		
			const TSharedPtr<FJsonObject>* StylesObject = nullptr;
			if (ObjectPtr->TryGetObjectField(TEXT("Styles"), StylesObject))
			{
				InOutTheme.ObjectData.SetJsonObject(StylesObject->ToSharedRef());
			}
		}
	}
}

bool UStarshipOverrideStyleThemeManager::ReadTheme(const FString& InThemeFileData, FStarshipOverrideStyleTheme& InOutTheme)
{
	TSharedRef<TJsonReader<>> ReaderRef = TJsonReaderFactory<>::Create(InThemeFileData);
	TJsonReader<>& Reader = ReaderRef.Get();

	TSharedPtr<FJsonObject> ObjectPtr;
	if (FJsonSerializer::Deserialize(Reader, ObjectPtr) && ObjectPtr.IsValid())
	{
		int32 Version = 0;
		if (!ObjectPtr->TryGetNumberField("Version", Version))
		{
			// Invalid file
			return false;
		}

		FString IdString;
		if (!ObjectPtr->TryGetStringField(TEXT("Id"), IdString) || !FGuid::Parse(IdString, InOutTheme.Id))
		{
			// Invalid Id;
			return false;
		}

		FString DisplayStr;
		if (!ObjectPtr->TryGetStringField(TEXT("DisplayName"), DisplayStr))
		{
			// Invalid file
			return false;
		}
		InOutTheme.DisplayName = FText::FromString(MoveTemp(DisplayStr));

		// Just check that the theme has colors. We wont load them unless the theme is used
		if (!ObjectPtr->HasField("Styles"))
		{
			// No colors
			return false;
		}
	}
	else
	{
		// Log invalid style file
		return false;
	}

	return true;
}

void UStarshipOverrideStyleThemeManager::CalcStyleDefines()
{
	TArray<UScriptStruct*> StyleWidgetStructList;
	TSet<UScriptStruct*> Visited;
	UPackage* TransientPackage = GetTransientPackage();

	for (TObjectIterator<UScriptStruct> StructIt; StructIt; ++StructIt)
	{
		UScriptStruct* CurrentStruct = *StructIt;
		UScriptStruct* Original = CurrentStruct;
		if (Visited.Contains(CurrentStruct) || CurrentStruct->GetOutermost() == TransientPackage)
		{
			// Skip transient structs as they are dead leftovers from user struct editing
			continue;
		}

		TArray<UScriptStruct*> TempVisited;

		while (CurrentStruct)
		{
			UScriptStruct* SuperStruct = Cast<UScriptStruct>(CurrentStruct->GetSuperStruct());

			if (!Visited.Contains(CurrentStruct))
			{
				TempVisited.Add(CurrentStruct);
			}

			if (FSlateWidgetStyle::StaticStruct() == SuperStruct)
			{
				StyleWidgetStructList.Append(TempVisited);
				Visited.Append(MoveTemp(TempVisited));
				break;
			}


			CurrentStruct = SuperStruct;
		}
	}

	TMap<FName, TWeakObjectPtr<UScriptStruct>> SlateWidgetStyleList;

	for (UScriptStruct* StyleStruct : StyleWidgetStructList)
	{
		uint8* RawData = (uint8*)FMemory::Malloc(StyleStruct->GetStructureSize());
		StyleStruct->InitializeStruct(RawData);

		FSlateWidgetStyle* WidgetStyle = (FSlateWidgetStyle*)RawData;
		SlateWidgetStyleList.Add(WidgetStyle->GetTypeName(), StyleStruct);

		StyleStruct->DestroyStruct(RawData);
		FMemory::Free(RawData);
	}

	TMap<FName, const FSlateWidgetStyle*> DefaultWidgetValueMap;
	TMap<FName, const FSlateBrush*> DefaultBrushValueMap;
	TMap<FName, FVector2D> DefaultVectorValueMap;
	TMap<FName, float> DefaultFloatValueMap;
	TMap<FName, const FLinearColor*> DefaultLinearColorValueMap;
	TMap<FName, FSlateColor> DefaultSlateColorValueMap;
	TMap<FName, const FMargin*> DefaultMarginValueMap;

	const UStarshipOverrideStyleSettings* StyleSetting = GetMutableDefault<UStarshipOverrideStyleSettings>();
	const bool bEnableEditOtherStyleType = StyleSetting->bEnableEditOtherStyleType;

	auto GatherWidgetDefines = [&](const FName& StyleName)
	{
		const ISlateStyle* CoreStyle = FSlateStyleRegistry::FindSlateStyle(StyleName);
		TSet<FName> CoreStyleKeyList = CoreStyle->GetStyleKeys();
		const FSlateGetWidget SlateGetWidget(StyleName);

		const FSlateGetWidget* IgnoreSlateStyleSet = &SlateGetWidget;

		const FVector2D DefaultVector(-1.0f, -1.0f);
		const FLinearColor DefaultColor(-1.0f, -1.0f, -1.0f);
		const FSlateColor DefaultSlateColor(DefaultColor);
		const FMargin DefaultMargin(-1.0f);
		const float DefaultFloat(-1.0f);

		for (const FName& StyleSetName : CoreStyleKeyList)
		{
			if (bEnableEditOtherStyleType)
			{
				if (float FloatStyleValue = CoreStyle->GetFloat(StyleSetName, nullptr, DefaultFloat, IgnoreSlateStyleSet); FloatStyleValue != DefaultFloat)
				{
					StyleWidgetDefines.FloatDefineList.Add(StyleSetName);
					DefaultFloatValueMap.Add(StyleSetName, FloatStyleValue);
					continue;
				}
				else if (FVector2D VectorStyleValue = CoreStyle->GetVector(StyleSetName, nullptr, DefaultVector, IgnoreSlateStyleSet); VectorStyleValue != DefaultVector)
				{
					StyleWidgetDefines.Vector2DDefineList.Add(StyleSetName);
					DefaultVectorValueMap.Add(StyleSetName, VectorStyleValue);
					continue;
				}
				else if (const FLinearColor& LinearColorStyleValue = CoreStyle->GetColor(StyleSetName, nullptr, DefaultColor, IgnoreSlateStyleSet); LinearColorStyleValue != DefaultColor)
				{
					StyleWidgetDefines.LinearColorDefineList.Add(StyleSetName);
					DefaultLinearColorValueMap.Add(StyleSetName, &LinearColorStyleValue);	// Maybe set LinearColorStyleValue pointer.
					continue;
				}
#if ENABLE_STARSHIP_OVERRIDE_SLATE_COLOR
				// Most SlateColor cannot set the color properly because it is processed by Color Theme.
				else if (const FSlateColor SlateColorStyleValue = CoreStyle->GetSlateColor(StyleSetName, nullptr, DefaultSlateColor, IgnoreSlateStyleSet); SlateColorStyleValue != DefaultSlateColor)
				{
					StyleWidgetDefines.SlateColorDefineList.Add(StyleSetName);
					DefaultSlateColorValueMap.Add(StyleSetName, SlateColorStyleValue);
					continue;
				}
#endif
				else if (const FMargin& MarginStyleValue = CoreStyle->GetMargin(StyleSetName, nullptr, DefaultMargin, IgnoreSlateStyleSet); MarginStyleValue != DefaultMargin)
				{
					StyleWidgetDefines.MarginDefineList.Add(StyleSetName);
					DefaultMarginValueMap.Add(StyleSetName, &MarginStyleValue);
					continue;
				}
			}


			if (const FSlateBrush* FoundBrush = CoreStyle->GetOptionalBrush(StyleSetName, nullptr, nullptr))
			{
				StyleWidgetDefines.BrushDefineList.Add(StyleSetName);
				DefaultBrushValueMap.Add(StyleSetName, FoundBrush);
			}
			else
			{
				// Calc Widget
				for (TPair<FName, TWeakObjectPtr<UScriptStruct>>& KeyValue : SlateWidgetStyleList)
				{
					if (!KeyValue.Value.IsValid())
					{
						continue;
					}

					const FSlateWidgetStyle* WidgetStyle = SlateGetWidget.GetWidgetStyle(KeyValue.Key, StyleSetName);
					if (WidgetStyle != nullptr)
					{
						StyleWidgetDefines.StyleWidgetDefineList.Add({ KeyValue.Value, StyleSetName, KeyValue.Key });
						DefaultWidgetValueMap.Add(StyleSetName, WidgetStyle);
						break;
					}
				}
			}
		}
	};


	GatherWidgetDefines("CoreStyle");
	GatherWidgetDefines("EditorStyle");

	DefaultEditorStyleData.Create({ StyleWidgetDefines, DefaultWidgetValueMap, DefaultBrushValueMap, DefaultFloatValueMap, DefaultVectorValueMap, DefaultLinearColorValueMap, DefaultSlateColorValueMap, DefaultMarginValueMap });
}

void UStarshipOverrideStyleThemeManager::CreateDiffJsonObject(FJsonObject& OutNewJsonObject, const FJsonObject& InDiffJsonObject, const FJsonObject& InBaseJsonObject)
{
	for (const TPair<FString, TSharedPtr<FJsonValue>>& EditedJsonObjectKeyValue : InDiffJsonObject.Values)
	{
		TSharedPtr<FJsonValue> FoundDefaultValue = InBaseJsonObject.TryGetField(EditedJsonObjectKeyValue.Key);

		if (FoundDefaultValue)
		{

			const FJsonValue& Lhs = *EditedJsonObjectKeyValue.Value.Get();
			const FJsonValue& Rhs = *FoundDefaultValue.Get();

			if (Lhs.Type != Rhs.Type)
			{
				OutNewJsonObject.SetField(EditedJsonObjectKeyValue.Key, EditedJsonObjectKeyValue.Value);
			}
			else
			{
				switch (Lhs.Type)
				{
				case EJson::None:
				case EJson::Null:
				case EJson::String:
				case EJson::Number:
				case EJson::Boolean:
					if (!FJsonValue::CompareEqual(Lhs, Rhs))
					{
						OutNewJsonObject.SetField(EditedJsonObjectKeyValue.Key, EditedJsonObjectKeyValue.Value);
					}
					break;

				case EJson::Array:
				{
					const TArray< TSharedPtr<FJsonValue> >& LhsArray = Lhs.AsArray();
					const TArray< TSharedPtr<FJsonValue> >& RhsArray = Rhs.AsArray();

					bool bShouldAddJsonValue = LhsArray.Num() != RhsArray.Num();
					if (!bShouldAddJsonValue)
					{
						// compare each element
						for (int32 i = 0; i < LhsArray.Num(); ++i)
						{
							if (!FJsonValue::CompareEqual(*LhsArray[i], *RhsArray[i]))
							{
								bShouldAddJsonValue = true;
								break;
							}
						}
					}

					if (bShouldAddJsonValue)
					{
						OutNewJsonObject.SetArrayField(EditedJsonObjectKeyValue.Key, RhsArray);
					}
					bShouldAddJsonValue = false;
				}
				break;

				case EJson::Object:
				{
					const TSharedPtr<FJsonObject>& LhsObject = Lhs.AsObject();
					const TSharedPtr<FJsonObject>& RhsObject = Rhs.AsObject();

					TSharedRef<FJsonObject> CandidateJsonObject = MakeShared<FJsonObject>();
					if (LhsObject.IsValid() != RhsObject.IsValid())
					{
						if (RhsObject.IsValid())
						{
							OutNewJsonObject.SetObjectField(EditedJsonObjectKeyValue.Key, RhsObject);
						}
					}
					else
					{
						CreateDiffJsonObject(CandidateJsonObject.Get(), *LhsObject, *RhsObject);

						if (CandidateJsonObject->Values.Num() > 0 && LhsObject->HasField("StarshipOverride_ColorData"))
						{
							CandidateJsonObject = LhsObject.ToSharedRef();
						}
					}

					if (CandidateJsonObject->Values.Num() > 0)
					{
						OutNewJsonObject.SetObjectField(EditedJsonObjectKeyValue.Key, CandidateJsonObject);
					}
				}
				break;

				default:
					break;
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE