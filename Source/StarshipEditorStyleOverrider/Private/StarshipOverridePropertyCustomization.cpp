// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverridePropertyCustomization.h"

#if 0
#include "PropertyEditor/Public/DetailCategoryBuilder.h"
#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "StarshipOverrideStyleCustomization.h"

TSharedRef<IDetailCustomization> FStarshipOverridePropertyCustomization::MakeInstance(TWeakPtr<SStarshipOverrideStyleEditor> StyleEditor)
{
	return MakeShared<FStarshipOverridePropertyCustomization>(StyleEditor);
}

FStarshipOverridePropertyCustomization::FStarshipOverridePropertyCustomization(const TWeakPtr<SStarshipOverrideStyleEditor>& InStyleEditor)
	: StyleEditor(InStyleEditor)
{

}

void FStarshipOverridePropertyCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& ColorCategory = DetailLayout.EditCategory("Style Parameters");

	TArray<UObject*> ObjectList;
	ObjectList.Add(StyleEditor.Pin()->GetObjectData()->GetObject());
	IDetailPropertyRow* PropertyRow = ColorCategory.AddExternalObjectProperty(ObjectList, StyleEditor.Pin()->GetCurrentSelectedStyleSetName());
	PropertyRow->ShouldAutoExpand(true);
}

#endif