// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailPropertyExtensionHandler.h"
#include "StarshipEditorStyleOverriderDefines.h"


class SStarshipOverrideStyleEditor;

class FStarshipOverrideStyleRowExtensionHandler : public IDetailPropertyExtensionHandler, public TSharedFromThis<FStarshipOverrideStyleRowExtensionHandler>
{
public:
	FStarshipOverrideStyleRowExtensionHandler(const TSharedRef<SStarshipOverrideStyleEditor>& InStyleEditor);

	bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& PropertyHandle) const override;

	void ExtendWidgetRow(FDetailWidgetRow& InOutWidgetRow, const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass, TSharedPtr<IPropertyHandle> PropertyHandle) override;

	void SetCurrentStatus(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName);
	void RefreshComparisonData();

private:
	bool CanSetDefaultEditorValue(FString InPropertyPath) const;
	void SetDefaultEditorValue(FString InPropertyPath);

	struct FExtensionData
	{
		TSharedPtr<IPropertyHandle> PropertyHandle;
		bool CanSetDefaultEditorValue;
	};

	TWeakPtr<SStarshipOverrideStyleEditor> StyleEditor;
	TMap<FString, FExtensionData> CanSetDefaultEditorValueMap;
	FName CurrentStyleSetName;
	EStarshipOverrideHandlerCategory CurrentCategoryType;
};

