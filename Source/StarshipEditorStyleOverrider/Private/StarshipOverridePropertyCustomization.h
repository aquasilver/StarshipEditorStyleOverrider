// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if 0
#include "IDetailCustomization.h"

class SStarshipOverrideStyleEditor;

class FStarshipOverridePropertyCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(TWeakPtr<SStarshipOverrideStyleEditor> InStyleEditor);

	FStarshipOverridePropertyCustomization(const TWeakPtr<SStarshipOverrideStyleEditor>& InStyleEditor);

	void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:

	TWeakPtr<SStarshipOverrideStyleEditor> StyleEditor;
};

#endif