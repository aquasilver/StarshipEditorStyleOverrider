// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StarshipOverrideStyleSettings.generated.h"

UCLASS(config=EditorPerProjectUserSettings)
class UStarshipOverrideStyleSettings : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	void Init();

	UPROPERTY(config)
	FGuid CurrentThemeId;

	UPROPERTY(Config, EditAnywhere, Category=EditorConfig, Meta=(DisplayName="Enable Real Time Editing (Beta)"))
	bool bEnableRealTimeEditing;

	UPROPERTY(Config, EditAnywhere, Category=EditorConfig, meta = (ConfigRestartRequired = true, ToolTip = "StarshipOverrider Style should be applied once when UE starts."))
	bool bIsStyleNotAppliedAfterStartup;

	UPROPERTY(EditAnywhere, config, Category=EditorConfig, meta=(ConfigRestartRequired=true, ToolTip = "Allows editing of styles such as Float and Vector2D."))
	bool bEnableEditOtherStyleType;
};