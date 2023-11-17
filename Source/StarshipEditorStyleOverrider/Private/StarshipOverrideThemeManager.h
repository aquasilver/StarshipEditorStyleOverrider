// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/StyleColors.h"
#include "StarshipOverrideStyleObjectData.h"
#include "StarshipOverrideThemeManager.generated.h"

struct FSlateWidgetStyle;

struct FStarshipOverrideStyleTheme
{
	FGuid Id;
	FText DisplayName;
	FString Filename;
	FStarshipOverrideStyleRawPtrData ObjectData;

	bool operator==(const FStarshipOverrideStyleTheme& InOther) const
	{
		return Id == InOther.Id;
	}

	bool operator==(const FGuid& InOtherId) const
	{
		return Id == InOtherId;
	}
};

struct FStarshipOverrideStyleDefines
{
	struct WidgetDefine
	{
		TWeakObjectPtr<UScriptStruct> SlateWidgetType;
		FName StyleSetName;
		FName DesiredTypeName;
	};

	TArray<WidgetDefine> StyleWidgetDefineList;
	TArray<FName> BrushDefineList;
	TArray<FName> FloatDefineList;
	TArray<FName> Vector2DDefineList;
	TArray<FName> LinearColorDefineList;
	TArray<FName> SlateColorDefineList;
	TArray<FName> MarginDefineList;
};

UCLASS()
class UStarshipOverrideStyleThemeManager : public UObject
{
	// This class referred to USlateThemeManager.

	GENERATED_BODY()
public:
	static UStarshipOverrideStyleThemeManager& Get()
	{
		return *GetMutableDefault<UStarshipOverrideStyleThemeManager>();
	}

	UStarshipOverrideStyleThemeManager();

	void LoadThemes();

	void SaveCurrentThemeAs(const FString& InFilename, const FStarshipOverrideStyleRawPtrData& InSaveData);
	void ApplyTheme(const FGuid& InThemeId);
	void ApplyDefaultTheme();
	void RemoveTheme(const FGuid& InThemeId);
	FGuid DuplicateActiveTheme();
	void ValidateActiveTheme();


	void SetCurrentThemeDisplayName(const FText& InNewDisplayName);
	const FStarshipOverrideStyleTheme& GetCurrentTheme() const { return *Themes.FindByKey(CurrentThemeId); }

	const TArray<FStarshipOverrideStyleTheme>& GetThemes() const { return Themes; }
	const FStarshipOverrideStyleDefines& GetStyleDefines() const { return StyleWidgetDefines; }
	const FStarshipOverrideStyleRawPtrData& GetDefaultEditorStyleData() const { return DefaultEditorStyleData; }

	FString GetProjectThemeDir() const;
	FString GetUserThemeDir() const;

	bool IsCurrentDefaultTheme() const { return DefaultThemeId == CurrentThemeId; }


	DECLARE_EVENT_OneParam(UStarshipOverrideStyleThemeManager, FThemeChangedEvent, const FGuid&);
	DECLARE_EVENT_OneParam(UStarshipOverrideStyleThemeManager, FThemeUpdatedEvent, const FGuid&);
	FThemeChangedEvent& OnThemeChanged() { return ThemeChangedEvent; }
	FThemeUpdatedEvent& OnThemeUpdated() { return ThemeUpdatedEvent; }

private:
	FStarshipOverrideStyleTheme& GetMutableCurrentTheme() { return *Themes.FindByKey(CurrentThemeId); }

	void LoadThemesFromDirectory(const FString& InDirectory);
	void LoadThemeStyles(FStarshipOverrideStyleTheme& InOutTheme);
	bool ReadTheme(const FString& InThemeFileData, FStarshipOverrideStyleTheme& OutTheme);
	void EnsureValidCurrentTheme();

	void CalcStyleDefines();

	static void CreateDiffJsonObject(FJsonObject& OutNewJsonObject, const FJsonObject& InDiffJsonObject, const FJsonObject& InBaseJsonObject);

	TArray<FStarshipOverrideStyleTheme> Themes;

	FStarshipOverrideStyleDefines StyleWidgetDefines;
	FStarshipOverrideStyleRawPtrData DefaultEditorStyleData;

	FGuid DefaultThemeId;
	FGuid CurrentThemeId;

	FThemeChangedEvent ThemeChangedEvent;
	FThemeUpdatedEvent ThemeUpdatedEvent;

	static const FString ThemesSubDir;
};