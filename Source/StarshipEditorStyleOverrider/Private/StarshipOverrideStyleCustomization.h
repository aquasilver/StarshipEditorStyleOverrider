// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IDetailPropertyExtensionHandler.h"
#include "StarshipOverrideStyleObjectData.h"


class IDetailPropertyRow;
class SWindow;
class SKismetInspector;
class SStarshipEditorStyleWidgetTree;
class STextComboBox;
class SEditableTextBox;
class FDetailTreeNode;
class SStarshipOverrideStyleEditor;
class FStarshipOverrideStyleRowExtensionHandler;

DECLARE_DELEGATE_OneParam(FOnStarshipOverrideEditorClosed, bool)
class SStarshipOverrideStyleEditor : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SStarshipOverrideStyleEditor) {}

		SLATE_EVENT(FOnStarshipOverrideEditorClosed, OnEditorClosed);

		SLATE_ATTRIBUTE(FString, DisplayName)
		SLATE_ATTRIBUTE(FString, OriginalThemeName)
	SLATE_END_ARGS()

public:
	SStarshipOverrideStyleEditor();
	~SStarshipOverrideStyleEditor();

	void Construct(const FArguments& InArgs, TSharedRef<SWindow> InParentWindow);

	void OnSelectionChanged(const FName& InStyleSetName, EStarshipOverrideHandlerCategory InCategoryType);

	const FName& GetCurrentSelectedStyleSetName() const { return CurrentSelectedStyleSetName; }

	bool CanSetDefaultEditorValue(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const;
	void SetDefaultEditorValue(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName);

private:

	FReply OnCancelClicked();

	void OnParentWindowClosed(const TSharedRef<SWindow>&);

	FReply OnSaveClicked();

	void CreateSlateWidgetStyleEditData();

	bool ValidateThemeName(const FText& InThemeName);
	FText GetThemeName() const;
	void OnThemeNameChanged(const FText& InNewName);
	void OnThemeNameCommitted(const FText& InNewName, ETextCommit::Type = ETextCommit::Default);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& InPropertyChangedEvent);

	void OnPreTickSlate(float InDeltaTime);

private:
	FOnStarshipOverrideEditorClosed OnEditorClosed;
	TWeakPtr<SWindow> ParentWindow;

	TSharedPtr<SKismetInspector> StyleInspector;
	TSharedPtr<SEditableTextBox> EditableThemeName; 
	TSharedPtr<FStarshipOverrideStyleRowExtensionHandler> RowExtensionHandler;

	FStarshipOverrideStyleObjectData WidgetStyleEditObjectData;
	FStarshipOverrideStyleObjectData DefaultThemeObjectData;
	FStarshipOverrideStyleRawPtrData DefaultEditorStyleData;

	TSharedPtr<SStarshipEditorStyleWidgetTree> StyleTree;
	TArray<TObjectPtr<const UObject>> RequestToSynchronizeStyleObjects;

	TSharedPtr<FStarshipOverrideStyleObjectData::FRootClassObjectController> RootClassObjectsReference;

	TAttribute<FString> ThemeDisplayNameAttribute;
	TAttribute<FString> ThemeOriginalNameAttribute;
	FName CurrentSelectedStyleSetName;
};


class FStarshipOverrideStyleCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override;

	void RefreshComboBox();

private:
	void GenerateThemeOptions(TSharedPtr<FString>& OutSelectedTheme);
	void MakeThemePickerRow(IDetailPropertyRow& OutPropertyRow);
	void OpenThemeEditorWindow(const FOnStarshipOverrideEditorClosed& InOnThemeEditorClosed);

	FReply OnExportThemeClicked(); 
	FReply OnImportThemeClicked(); 
	FReply OnDeleteThemeClicked();
	FReply OnDuplicateAndEditThemeClicked();
	FReply OnEditThemeClicked();
	FString GetTextLabelForThemeEntry(TSharedPtr<FString> InEntry);
	void OnThemePicked(TSharedPtr<FString> InNewSelection, ESelectInfo::Type InSelectInfo);
	bool IsThemeEditingEnabled() const;

	static TWeakPtr<SWindow> ThemeEditorWindow;


	FString CurrentActiveThemeDisplayName;
	FString OriginalThemeName;

	TArray<TSharedPtr<FString>> ThemeOptions;
	TSharedPtr<STextComboBox> ComboBox;
};
