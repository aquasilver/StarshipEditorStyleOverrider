// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyleSettings.h"
#include "StarshipOverrideThemeManager.h"

/* UStarshipOverrideStyleSettings
 *****************************************************************************/
UStarshipOverrideStyleSettings::UStarshipOverrideStyleSettings(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer)
	, bEnableRealTimeEditing(true)
	, bIsStyleNotAppliedAfterStartup(false)
	, bEnableEditOtherStyleType(false)
{
	
}

void UStarshipOverrideStyleSettings::Init()
{
	if (CurrentThemeId.IsValid())
	{
		UStarshipOverrideStyleThemeManager::Get().ApplyTheme(CurrentThemeId);
	}
}
