// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyleCustomization.h"
#include "StarshipOverrideStyleSettings.h"
#include "StarshipOverridePropertyCustomization.h"
#include "StarshipOverrideStyleRowExtensionHandler.h"
#include "StarshipOverrideThemeManager.h"
#include "StarshipEditorStyleWidgetTree.h"
#include "StarshipOverrideStyle.h"
#include "DetailLayoutBuilder.h"
#include "SSimpleButton.h"
#include "SKismetInspector.h"
#include "SPrimaryButton.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Misc/MessageDialog.h"




TWeakPtr<SWindow> FStarshipOverrideStyleCustomization::ThemeEditorWindow;

#define LOCTEXT_NAMESPACE "StarshipEditorStyleOverrider"

/* SStarshipEditorStyleOverrideEditor
*****************************************************************************/
SStarshipOverrideStyleEditor::SStarshipOverrideStyleEditor()
{

}

SStarshipOverrideStyleEditor::~SStarshipOverrideStyleEditor()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnPreTick().RemoveAll(this);
	}
}

void SStarshipOverrideStyleEditor::Construct(const FArguments& InArgs, TSharedRef<SWindow> InParentWindow)
{
	FSlateApplication::Get().OnPreTick().AddSP(this, &SStarshipOverrideStyleEditor::OnPreTickSlate);
	ThemeDisplayNameAttribute = InArgs._DisplayName;
	ThemeOriginalNameAttribute = InArgs._OriginalThemeName;
	OnEditorClosed = InArgs._OnEditorClosed;
	ParentWindow = InParentWindow;
	InParentWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &SStarshipOverrideStyleEditor::OnParentWindowClosed));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	ChildSlot[
		SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
			[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(6.0f, 3.0f)
					.AutoHeight()
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(0.6f)
							.HAlign(HAlign_Right)
							.VAlign(VAlign_Center)
							.Padding(5.0f, 2.0f)
							[
								SNew(STextBlock)
									.Text(LOCTEXT("ThemeName", "Name"))
							]
							+ SHorizontalBox::Slot()
							.FillWidth(2.0f)
							.VAlign(VAlign_Center)
							.Padding(5.0f, 2.0f)
							[
								SAssignNew(EditableThemeName, SEditableTextBox)
									.Text(this, &SStarshipOverrideStyleEditor::GetThemeName)
									.OnTextChanged(this, &SStarshipOverrideStyleEditor::OnThemeNameChanged)
									.OnTextCommitted(this, &SStarshipOverrideStyleEditor::OnThemeNameCommitted)
									.SelectAllTextWhenFocused(true)
							]
					]
					+ SVerticalBox::Slot()
					[
						SNew(SSplitter)
							+SSplitter::Slot()
							.Value(0.35f)
							[
								SAssignNew(StyleTree, SStarshipEditorStyleWidgetTree, SharedThis(this))
							]

							+ SSplitter::Slot()
							[
								SNew(SScrollBox)
									+ SScrollBox::Slot()
									[
										SAssignNew(StyleInspector, SKismetInspector)
											.ShowTitleArea(false)
											.OnFinishedChangingProperties(FOnFinishedChangingProperties::FDelegate::CreateSP(this, &SStarshipOverrideStyleEditor::OnFinishedChangingProperties))
									]
							]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					//.Padding(6.0f, 3.0f)
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Bottom)
							.Padding(4, 3)
							[
								SNew(SPrimaryButton)
									.Text(LOCTEXT("SaveThemeButton", "Save"))
									.OnClicked(this, &SStarshipOverrideStyleEditor::OnSaveClicked)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Bottom)
							.Padding(4, 3)
							[
								SNew(SButton)
									.Text(LOCTEXT("CancelThemeEditingButton", "Cancel"))
									.OnClicked(this, &SStarshipOverrideStyleEditor::OnCancelClicked)
							]
					]
			]
	];

	CreateSlateWidgetStyleEditData();

	RowExtensionHandler = MakeShared<FStarshipOverrideStyleRowExtensionHandler>(SharedThis(this)).ToSharedPtr();
	StyleInspector->GetPropertyView()->SetExtensionHandler(RowExtensionHandler.ToSharedRef());
	//StyleInspector->GetPropertyView()->OnFinishedChangingProperties().AddSP(this, &SStarshipOverrideStyleEditor::OnFinishedChangingProperties);
	//FOnGetDetailCustomizationInstance LayoutVariableDetails = FOnGetDetailCustomizationInstance::CreateStatic(&FStarshipOverridePropertyCustomization::MakeInstance, SharedThis(this).ToWeakPtr());
	//StyleInspector->GetPropertyView()->RegisterInstancedCustomPropertyLayout(UPropertyWrapper::StaticClass(), LayoutVariableDetails);
}

void SStarshipOverrideStyleEditor::OnSelectionChanged(const FName& InStyleSetName, EStarshipOverrideHandlerCategory InCategoryType)
{
	CurrentSelectedStyleSetName = InStyleSetName;
	UObject* FoundObject = WidgetStyleEditObjectData.FindObject(InCategoryType, InStyleSetName);

	if (!FoundObject)
	{
		const FStarshipOverrideStyleRawDataBaseHandler* FoundHandler = UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme().ObjectData.FindHandler(InCategoryType, InStyleSetName);

		if (FoundHandler)
		{
			DefaultThemeObjectData.AddSpecificUObject(*FoundHandler, InCategoryType, InStyleSetName, true);
			DefaultThemeObjectData.UnlinkColors(InCategoryType, InStyleSetName);

			const UObject* DefaultNewObject = DefaultThemeObjectData.FindObject(InCategoryType, InStyleSetName);

			WidgetStyleEditObjectData.AddCopySpecificUobject(*DefaultNewObject);
			FoundObject = WidgetStyleEditObjectData.FindObject(InCategoryType, InStyleSetName);
		}
	}

	if (FoundObject)
	{
		RowExtensionHandler->SetCurrentStatus(InCategoryType, CurrentSelectedStyleSetName);

		SKismetInspector::FShowDetailsOptions Options;
		Options.bForceRefresh = true;
		
		StyleInspector->ShowDetailsForSingleObject(FoundObject, Options);

	}
	else
	{
		StyleInspector->ShowSingleStruct(TSharedPtr<FStructOnScope>());
	}
}

bool SStarshipOverrideStyleEditor::CanSetDefaultEditorValue(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const
{
	const void* SrcPtr = InPropertyHandle->GetValueBaseAddress((uint8*)WidgetStyleEditObjectData.FindObject(InCategoryType, InStyleSetName));

	// Check DefaultThemeObject value and EditObjectData.
	// Normal reset is handled by ResetToDefault of SDetailSingleItemRow.
	// If normal reset can be performed, there is no need to display SetDefaultEditorValue.
	{
		if (!InPropertyHandle->HasMetaData("NoResetToDefault") && !InPropertyHandle->GetInstanceMetaData("NoResetToDefault") && InPropertyHandle->CanResetToDefault())
		{
			return false;
		}
	}

	return !DefaultEditorStyleData.IsEqualValueFromPropertyHandle(InPropertyHandle, SrcPtr);
}

void SStarshipOverrideStyleEditor::SetDefaultEditorValue(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName)
{
	WidgetStyleEditObjectData.CopyDataFromProperty(InPropertyHandle, InCategoryType, InStyleSetName, DefaultEditorStyleData);
	StyleInspector->GetPropertyView()->ForceRefresh();
}

FReply SStarshipOverrideStyleEditor::OnCancelClicked()
{
	ParentWindow.Pin()->SetOnWindowClosed(FOnWindowClosed());
	ParentWindow.Pin()->RequestDestroyWindow();

	OnEditorClosed.ExecuteIfBound(false);
	return FReply::Handled();
}

void SStarshipOverrideStyleEditor::OnParentWindowClosed(const TSharedRef<SWindow>&)
{
	OnCancelClicked();
}

FReply SStarshipOverrideStyleEditor::OnSaveClicked()
{
	FString Filename;
	bool bSuccess = true;
	FString PreviousFilename;

	const FStarshipOverrideStyleTheme& Theme = UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme();

	// updated name is taken: DO NOT SAVE. 
	if (!ValidateThemeName(EditableThemeName->GetText()))
	{
		bSuccess = false;
	}
	// Duplicating a theme: 
	else if (Theme.Filename.IsEmpty())
	{
		// updated name is not taken: SAVE. 
		UStarshipOverrideStyleThemeManager::Get().SetCurrentThemeDisplayName(EditableThemeName->GetText());
		Filename = UStarshipOverrideStyleThemeManager::Get().GetUserThemeDir() / Theme.DisplayName.ToString() + TEXT(".json");
		EditableThemeName->SetError(FText::GetEmpty());
	}
	// Modifying a theme: would only be here if the user is modifying a user-specific theme. 
	else
	{
		// updated name is not taken: SAVE. 
		PreviousFilename = Theme.Filename;
		UStarshipOverrideStyleThemeManager::Get().SetCurrentThemeDisplayName(EditableThemeName->GetText());
		Filename = UStarshipOverrideStyleThemeManager::Get().GetUserThemeDir() / Theme.DisplayName.ToString() + TEXT(".json");
		EditableThemeName->SetError(FText::GetEmpty());
	}

	if (!Filename.IsEmpty() && bSuccess)
	{

		// Create the data to be saved in RawPtr since only the necessary UObjects are created.
		FStarshipOverrideStyleRawPtrData TempEditData;
		TempEditData.Create(UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme().ObjectData);
		// Overwrite edit data with TempEditoData
		WidgetStyleEditObjectData.QueryObject([&TempEditData](const FName& InStyleSetName, UObject& InObject) -> bool
		{
			TempEditData.CopyFromUObject(InObject);
			return false;
		});

		TempEditData.UnlinkColors();

		UStarshipOverrideStyleThemeManager::Get().SaveCurrentThemeAs(Filename, TempEditData);
		// if user modified an existing user-specific theme name, delete the old one. 
		if (!PreviousFilename.IsEmpty() && !PreviousFilename.Equals(Filename))
		{
			IPlatformFile::GetPlatformPhysical().DeleteFile(*PreviousFilename);
		}
		EditableThemeName->SetError(FText::GetEmpty());

		ParentWindow.Pin()->SetOnWindowClosed(FOnWindowClosed());
		ParentWindow.Pin()->RequestDestroyWindow();

		OnEditorClosed.ExecuteIfBound(true);

		UStarshipOverrideStyleSettings* StyleSetting = GetMutableDefault<UStarshipOverrideStyleSettings>();
		StyleSetting->CurrentThemeId = Theme.Id;
		StyleSetting->SaveConfig();

	}
	return FReply::Handled();
}

void SStarshipOverrideStyleEditor::CreateSlateWidgetStyleEditData()
{
	check(StyleTree);

	// Because a large number of Brushes exist, it takes a little time to start up, so we improved the format to create only the necessary UObjects.
	//DefaultThemeObjectData.CreateUObjects(FStarshipOverrideStyle::GetFrontlineSlateStyleObject().Get(), true);
	//DefaultThemeObjectData.UnlinkColors();
	//WidgetStyleEditObjectData.DuplicateUObjects(DefaultThemeObjectData);
	DefaultEditorStyleData.Create(UStarshipOverrideStyleThemeManager::Get().GetDefaultEditorStyleData());
	DefaultEditorStyleData.UnlinkColors();

	RootClassObjectsReference = DefaultThemeObjectData.GetRootClassObjectController();
}

bool SStarshipOverrideStyleEditor::ValidateThemeName(const FText& InThemeName)
{
	FText OutErrorMessage;
	const TArray<FStarshipOverrideStyleTheme>& ThemeOptions = UStarshipOverrideStyleThemeManager::Get().GetThemes();

	if (InThemeName.IsEmpty())
	{
		OutErrorMessage = LOCTEXT("ThemeNameEmpty", "The theme name cannot be empty.");
		EditableThemeName->SetError(OutErrorMessage);
		return false;
	}

	const FString& ThemeDisplayName = ThemeDisplayNameAttribute.Get();

	for (const FStarshipOverrideStyleTheme& Theme : ThemeOptions)
	{
		// show error message whenever there's duplicate (and different from the previous name) 
		if (Theme.DisplayName.EqualTo(InThemeName) && !ThemeDisplayName.Equals(InThemeName.ToString()))
		{
			OutErrorMessage = FText::Format(LOCTEXT("RenameThemeAlreadyExists", "A theme already exists with the name '{0}'."), InThemeName);
			EditableThemeName->SetError(OutErrorMessage);
			return false;
		}
	}
	EditableThemeName->SetError(FText::GetEmpty());
	return true; 
}

FText SStarshipOverrideStyleEditor::GetThemeName() const
{
	return UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme().DisplayName;
}

void SStarshipOverrideStyleEditor::OnThemeNameChanged(const FText& InNewName)
{
	// verify duplicates before setting the display name. 
	ValidateThemeName(InNewName);
}

void SStarshipOverrideStyleEditor::OnThemeNameCommitted(const FText& InNewName, ETextCommit::Type /*= ETextCommit::Default*/)
{
	if (!ValidateThemeName(InNewName))
	{
		const FText OriginalTheme = FText::FromString(ThemeOriginalNameAttribute.Get()); 
		EditableThemeName->SetText(OriginalTheme);
		EditableThemeName->SetError(FText::GetEmpty());
	}
	else
	{
		EditableThemeName->SetText(InNewName);
	}
}

void SStarshipOverrideStyleEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& InPropertyChangedEvent)
{
	const UStarshipOverrideStyleSettings* Settings = GetMutableDefault<UStarshipOverrideStyleSettings>();
	if (Settings->bEnableRealTimeEditing)
	{
		for (int32 Index = 0; Index < InPropertyChangedEvent.GetNumObjectsBeingEdited(); Index++)
		{
			if (const UObject* Object = InPropertyChangedEvent.GetObjectBeingEdited(Index))
			{
				RequestToSynchronizeStyleObjects.AddUnique(Object);
			}
		}
	}
}

void SStarshipOverrideStyleEditor::OnPreTickSlate(float /*InDeltaTime*/)
{
	if (RequestToSynchronizeStyleObjects.Num() > 0)
	{
		for (const TObjectPtr<const UObject>& Object : RequestToSynchronizeStyleObjects)
		{
			if (IsValid(Object))
			{
				FStarshipOverrideStyle::GetFrontlineSlateStyleObject()->CopyFromUObject(*Object.Get());
			}
		}

		RowExtensionHandler->RefreshComparisonData();

		RequestToSynchronizeStyleObjects.Empty();
	}
}

/* FStarshipOverrideStyleCustomization
 *****************************************************************************/
TSharedRef<IDetailCustomization> FStarshipOverrideStyleCustomization::MakeInstance()
{
	TSharedRef<FStarshipOverrideStyleCustomization> Instance = MakeShared<FStarshipOverrideStyleCustomization>();

	return Instance;
}

void FStarshipOverrideStyleCustomization::CustomizeDetails(IDetailLayoutBuilder& OutDetailLayout)
{
	IDetailCategoryBuilder& ColorCategory = OutDetailLayout.EditCategory("Theme_StarshipOverride");

	UStarshipOverrideStyleSettings* Settings = GetMutableDefault<UStarshipOverrideStyleSettings>();
	TArray<UObject*> Objects = { Settings };

	if (IDetailPropertyRow* ThemeRow = ColorCategory.AddExternalObjectProperty(Objects, "CurrentThemeId"))
	{
		MakeThemePickerRow(*ThemeRow);
	}
}

void FStarshipOverrideStyleCustomization::RefreshComboBox()
{
	TSharedPtr<FString> SelectedTheme;
	GenerateThemeOptions(SelectedTheme);
	ComboBox->RefreshOptions();
	ComboBox->SetSelectedItem(SelectedTheme);
}

void FStarshipOverrideStyleCustomization::MakeThemePickerRow(IDetailPropertyRow& OutPropertyRow)
{
	TSharedPtr<FString> SelectedItem;
	GenerateThemeOptions(SelectedItem);

	// Make combo choices
	ComboBox =
		SNew(STextComboBox)
		.OptionsSource(&ThemeOptions)
		.InitiallySelectedItem(SelectedItem)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OnGetTextLabelForItem(this, &FStarshipOverrideStyleCustomization::GetTextLabelForThemeEntry)
		.OnSelectionChanged(this, &FStarshipOverrideStyleCustomization::OnThemePicked);

	FDetailWidgetRow& CustomWidgetRow = OutPropertyRow.CustomWidget(false);

	CustomWidgetRow.NameContent()
		[
			OutPropertyRow.GetPropertyHandle()->CreatePropertyNameWidget(LOCTEXT("ActiveThemeDisplayName", "Active Theme"))
		]
		.ValueContent()
		.MaxDesiredWidth(350.0f)
		[
			SNew(SHorizontalBox)
				.IsEnabled(this, &FStarshipOverrideStyleCustomization::IsThemeEditingEnabled)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
						.WidthOverride(125.f)
						[
							ComboBox.ToSharedRef()
						]
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoWidth()
				[
					SNew(SSimpleButton)
						.Icon(FAppStyle::Get().GetBrush("Icons.Edit"))
						.IsEnabled_Lambda(
						[]()
					{
						return !UStarshipOverrideStyleThemeManager::Get().IsCurrentDefaultTheme();
					})
						.OnClicked(this, &FStarshipOverrideStyleCustomization::OnEditThemeClicked)
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoWidth()
				[
					SNew(SSimpleButton)
						.Icon(FAppStyle::Get().GetBrush("Icons.Duplicate"))
						.OnClicked(this, &FStarshipOverrideStyleCustomization::OnDuplicateAndEditThemeClicked)
				]
				//+ SHorizontalBox::Slot()
				//.AutoWidth()
				//.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				//[
				//	// export button
				//	SNew(SSimpleButton)
				//		.Icon(FAppStyle::Get().GetBrush("Themes.Export"))
				//		.OnClicked(this, &FStarshipOverrideStyleCustomization::OnExportThemeClicked)
				//]
				//+ SHorizontalBox::Slot()
				//.AutoWidth()
				//.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				//[
				//	// import button
				//	SNew(SSimpleButton)
				//		.Icon(FAppStyle::Get().GetBrush("Themes.Import"))
				//		.OnClicked(this, &FStarshipOverrideStyleCustomization::OnImportThemeClicked)
				//]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoWidth()
				[
					// delete button
					SNew(SSimpleButton)
						.Icon(FAppStyle::Get().GetBrush("Icons.Delete"))
						.IsEnabled_Lambda(
						[]()
					{
						return !UStarshipOverrideStyleThemeManager::Get().IsCurrentDefaultTheme(); 
					})
						.OnClicked(this, &FStarshipOverrideStyleCustomization::OnDeleteThemeClicked)
				]
		];
}

void FStarshipOverrideStyleCustomization::GenerateThemeOptions(TSharedPtr<FString>& OutSelectedTheme)
{
	UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();
	const TArray<FStarshipOverrideStyleTheme>& Themes = ThemeManager.GetThemes();

	ThemeOptions.Empty(Themes.Num());
	int32 Index = 0;
	for (const FStarshipOverrideStyleTheme& Theme : Themes)
	{
		TSharedRef<FString> ThemeString = MakeShared<FString>(FString::FromInt(Index));

		if (ThemeManager.GetCurrentTheme() == Theme)
		{
			OutSelectedTheme = ThemeString;
		}

		ThemeOptions.Add(ThemeString);
		++Index;
	}
}

static void OnThemeEditorClosed(bool bSaved, TWeakPtr<FStarshipOverrideStyleCustomization> ActiveCustomization, FGuid CreatedThemeId, FGuid PreviousThemeId)
{
	if (!bSaved)
	{
		UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();
		if (PreviousThemeId.IsValid())
		{
			ThemeManager.ApplyTheme(PreviousThemeId);

			if (CreatedThemeId.IsValid())
			{
				ThemeManager.RemoveTheme(CreatedThemeId);

			}
			if (ActiveCustomization.IsValid())
			{
				ActiveCustomization.Pin()->RefreshComboBox();
			}
		}
		else
		{
			const UStarshipOverrideStyleSettings* Settings = GetMutableDefault<UStarshipOverrideStyleSettings>();
			if (!Settings->bIsStyleNotAppliedAfterStartup)
			{
				const FStarshipOverrideStyleTheme& Theme = UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme();
				FStarshipOverrideStyle::GetFrontlineSlateStyleObject()->CopyData(Theme.ObjectData);
			}
		}
	}
}

FReply FStarshipOverrideStyleCustomization::OnExportThemeClicked()
{
	return FReply::Handled();
}

void FStarshipOverrideStyleCustomization::OpenThemeEditorWindow(const FOnStarshipOverrideEditorClosed& InOnThemeEditorClosed)
{
	if(!ThemeEditorWindow.IsValid())
	{
		TSharedRef<SWindow> NewWindow = SNew(SWindow)
			.Title(LOCTEXT("ThemeEditorWindowTitle", "Starship Override Theme Editor"))
			.SizingRule(ESizingRule::UserSized)
			.ClientSize(FVector2D(1280, 720))
			.SupportsMaximize(false)
			.SupportsMinimize(false);

		TSharedRef<SStarshipOverrideStyleEditor> ThemeEditor =
			SNew(SStarshipOverrideStyleEditor, NewWindow)
			.OnEditorClosed(InOnThemeEditorClosed)
			.DisplayName(CurrentActiveThemeDisplayName)
			.OriginalThemeName(OriginalThemeName);

		NewWindow->SetContent(
			ThemeEditor
		);

		if (TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(ComboBox.ToSharedRef()))
		{
			FSlateApplication::Get().AddWindowAsNativeChild(NewWindow, ParentWindow.ToSharedRef());
		}
		else
		{
			FSlateApplication::Get().AddWindow(NewWindow);
		}

		ThemeEditorWindow = NewWindow;
	}

}

FReply FStarshipOverrideStyleCustomization::OnImportThemeClicked()
{
	return FReply::Handled();
}

FReply FStarshipOverrideStyleCustomization::OnDuplicateAndEditThemeClicked()
{
	UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();

	FGuid PreviouslyActiveTheme = ThemeManager.GetCurrentTheme().Id;

	FGuid NewThemeId = ThemeManager.DuplicateActiveTheme();
	ThemeManager.ApplyTheme(NewThemeId);
	// Set the new theme name to empty FText, to avoid a generated name collision or needing to delete a template name
	ThemeManager.SetCurrentThemeDisplayName(FText::GetEmpty());
	CurrentActiveThemeDisplayName = ThemeManager.GetCurrentTheme().DisplayName.ToString();
	OriginalThemeName = ThemeManager.GetCurrentTheme().DisplayName.ToString(); 

	RefreshComboBox();

	OpenThemeEditorWindow(FOnStarshipOverrideEditorClosed::CreateStatic(&OnThemeEditorClosed, TWeakPtr<FStarshipOverrideStyleCustomization>(SharedThis(this)), NewThemeId, PreviouslyActiveTheme));

	return FReply::Handled();
}

FReply FStarshipOverrideStyleCustomization::OnDeleteThemeClicked()
{
	UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();
	const FStarshipOverrideStyleTheme PreviouslyActiveTheme = ThemeManager.GetCurrentTheme();

	// Are you sure you want to do this?
	const FText FileNameToRemove = FText::FromString(PreviouslyActiveTheme.DisplayName.ToString());
	const FText TextBody = FText::Format(LOCTEXT("ActionRemoveMsg", "Are you sure you want to permanently delete the theme \"{0}\"? This action cannot be undone."), FileNameToRemove);
	const FText TextTitle = FText::Format(LOCTEXT("RemoveTheme_Title", "Remove Theme \"{0}\"?"), FileNameToRemove);

	// If user select "OK"...
#if ENGINE_MAJOR_VERSION >=	5 && ENGINE_MINOR_VERSION >= 3
	if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::OkCancel, TextBody, TextTitle))
#else
	if (EAppReturnType::Ok == FMessageDialog::Open(EAppMsgType::OkCancel, TextBody, &TextTitle))
#endif
	{
		// apply default theme
		ThemeManager.ApplyDefaultTheme();

		// remove previously active theme
		const FString Filename = ThemeManager.GetUserThemeDir() / PreviouslyActiveTheme.DisplayName.ToString() + TEXT(".json");
		IFileManager::Get().Delete(*Filename);
		ThemeManager.RemoveTheme(PreviouslyActiveTheme.Id);
		RefreshComboBox();
	}

	return FReply::Handled();
}

FReply FStarshipOverrideStyleCustomization::OnEditThemeClicked()
{
	const FGuid& CurrentlyActiveTheme = UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme().Id;

	CurrentActiveThemeDisplayName = UStarshipOverrideStyleThemeManager::Get().GetCurrentTheme().DisplayName.ToString();
	OriginalThemeName = CurrentActiveThemeDisplayName;

	OpenThemeEditorWindow(FOnStarshipOverrideEditorClosed::CreateStatic(&OnThemeEditorClosed, TWeakPtr<FStarshipOverrideStyleCustomization>(SharedThis(this)), FGuid(), CurrentlyActiveTheme));

	return FReply::Handled();
}

FString FStarshipOverrideStyleCustomization::GetTextLabelForThemeEntry(TSharedPtr<FString> InEntry)
{
	const TArray<FStarshipOverrideStyleTheme>& Themes = UStarshipOverrideStyleThemeManager::Get().GetThemes();
	return Themes[TCString<TCHAR>::Atoi(**InEntry)].DisplayName.ToString();
}

void FStarshipOverrideStyleCustomization::OnThemePicked(TSharedPtr<FString> InNewSelection, ESelectInfo::Type InSelectInfo)
{
	UStarshipOverrideStyleSettings* StyleSetting = GetMutableDefault<UStarshipOverrideStyleSettings>();

	// set current applied theme to selected theme. 
	const TArray<FStarshipOverrideStyleTheme>& Themes = UStarshipOverrideStyleThemeManager::Get().GetThemes();
	StyleSetting->CurrentThemeId = Themes[TCString<TCHAR>::Atoi(**InNewSelection)].Id;

	// If set directly in code, the theme was already applied
	if(InSelectInfo != ESelectInfo::Direct)
	{
		StyleSetting->SaveConfig();
		UStarshipOverrideStyleThemeManager::Get().ApplyTheme(StyleSetting->CurrentThemeId);
		//FStarshipOverrideStyle::GetFrontlineSlateStyleObject()->CopyData(UStarshipOverrideStyleThemeManager::)
	}
}

bool FStarshipOverrideStyleCustomization::IsThemeEditingEnabled() const
{
	// Don't allow changing themes while editing them
	return !ThemeEditorWindow.IsValid();
}



#undef LOCTEXT_NAMESPACE