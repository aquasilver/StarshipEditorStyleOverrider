// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class EStarshipOverrideHandlerCategory
{
	SlateWidgetStyle,
	Brush,
	Float,
	Vector2D,
	LinearColor,
	SlateColor,
	Margin,
	Count
};

#define ENABLE_STARSHIP_OVERRIDE_SLATE_COLOR 0