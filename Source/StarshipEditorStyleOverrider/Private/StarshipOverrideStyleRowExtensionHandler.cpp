// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyleRowExtensionHandler.h"
#include "StarshipOverrideStyleCustomization.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#if !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3)
#include "PropertyNode.h"
#endif


#define LOCTEXT_NAMESPACE "StarshipEditorStyleOverrider"

/* FStarshipOverrideStyleRowExtensionHandler
 *****************************************************************************/
FStarshipOverrideStyleRowExtensionHandler::FStarshipOverrideStyleRowExtensionHandler(const TSharedRef<SStarshipOverrideStyleEditor>& InStyleEditor)
	: StyleEditor(InStyleEditor)
	, CurrentCategoryType(EStarshipOverrideHandlerCategory::Float)
{

}

bool FStarshipOverrideStyleRowExtensionHandler::IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const
{
	return true;
}

void FStarshipOverrideStyleRowExtensionHandler::ExtendWidgetRow(FDetailWidgetRow& InOutWidgetRow, const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle)
{
	if (!StyleEditor.IsValid())
	{
		return;
	}
	ECheckBoxState InitialState = ECheckBoxState::Unchecked;

#if ENGINE_MAJOR_VERSION >=	5 && ENGINE_MINOR_VERSION >= 3
	const FString PropertyPath(InPropertyHandle->GetPropertyPath());
#else
	const FString& PropertyPath = InPropertyHandle->GetPropertyNode()->GetPropertyPath();
#endif
	CanSetDefaultEditorValueMap.Add(PropertyPath, { InPropertyHandle, StyleEditor.Pin()->CanSetDefaultEditorValue(InPropertyHandle.ToSharedRef(), CurrentCategoryType, CurrentStyleSetName) });
	//InDetailBuilder.GetPropertyUtilities()

	static FSlateIcon HasEditDataIcon(FAppStyle::Get().GetStyleSetName(), "StarshipOverride.HasEditData");


	InOutWidgetRow.AddCustomContextMenuAction(FUIAction(FExecuteAction::CreateSP(this, &FStarshipOverrideStyleRowExtensionHandler::SetDefaultEditorValue, PropertyPath)
		, FCanExecuteAction::CreateSP(this, &FStarshipOverrideStyleRowExtensionHandler::CanSetDefaultEditorValue, PropertyPath)),
		LOCTEXT("SetDefaultEditorValue", "Set Default Editor Value"),
		LOCTEXT("SetDefaultEditorValueTooltip", "Resets the selected item to its default value."),
		HasEditDataIcon);

	InOutWidgetRow.ExtensionContent()
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("StarshipOverride.HasEditData"))
				.Visibility_Lambda([this, PropertyPath]() -> EVisibility
			{
				if (FExtensionData* Data = CanSetDefaultEditorValueMap.Find(PropertyPath))
				{
					return Data->CanSetDefaultEditorValue ? EVisibility::Visible : EVisibility::Hidden;
				}
				
				return EVisibility::Hidden;
			})
				.ToolTipText(LOCTEXT("CanSetDefaultEditorValue", "This item can be reset to its initial value by selecting \"Set Default Editor Value\" from the context menu."))
		];

}

void FStarshipOverrideStyleRowExtensionHandler::SetCurrentStatus(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName)
{
	CurrentCategoryType = InCategoryType;
	CurrentStyleSetName = InStyleSetName;
	CanSetDefaultEditorValueMap.Empty();
}

void FStarshipOverrideStyleRowExtensionHandler::RefreshComparisonData()
{
	if (!StyleEditor.IsValid())
	{
		return;
	}
	const SStarshipOverrideStyleEditor* StyleEditorPtr = StyleEditor.Pin().Get();
	for (TPair<FString, FExtensionData>& KeyValue : CanSetDefaultEditorValueMap)
	{
		KeyValue.Value.CanSetDefaultEditorValue = StyleEditorPtr->CanSetDefaultEditorValue(KeyValue.Value.PropertyHandle.ToSharedRef(), CurrentCategoryType, CurrentStyleSetName);
	}
}

bool FStarshipOverrideStyleRowExtensionHandler::CanSetDefaultEditorValue(FString InPropertyPath) const
{
	return CanSetDefaultEditorValueMap.FindChecked(InPropertyPath).CanSetDefaultEditorValue;
}

void FStarshipOverrideStyleRowExtensionHandler::SetDefaultEditorValue(FString InPropertyPath)
{
	if (!StyleEditor.IsValid())
	{
		return;
	}

	StyleEditor.Pin()->SetDefaultEditorValue(CanSetDefaultEditorValueMap.FindChecked(InPropertyPath).PropertyHandle.ToSharedRef(), CurrentCategoryType, CurrentStyleSetName);
	//StyleEditor->FindObject(CurrentStyleSetName);
}

#undef LOCTEXT_NAMESPACE