// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "StarshipOverrideStyleObjectData.h"

class FStarshipOverrideStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static const FName& GetStyleSetName();

	static TSharedRef<FStarshipOverrideStyleRawPtrData> GetFrontlineSlateStyleObject();

	class FStyle : public FSlateStyleSet
	{
	public:
		FStyle();
		~FStyle();
		void Initialize();

		TSharedRef<FStarshipOverrideStyleRawPtrData> GetStyleObjectData() { return WidgetStylePropertyObjectData.ToSharedRef(); }

		float GetFloat(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, float InDefaultValue /* = FStyleDefaults::GetFloat() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;
		FVector2D GetVector(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, FVector2D InDefaultValue /* = FStyleDefaults::GetVector2D() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;
		const FSlateBrush* GetBrush(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;
		const FLinearColor& GetColor(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FLinearColor& InDefaultValue /* = FStyleDefaults::GetColor() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;
#if ENABLE_STARSHIP_OVERRIDE_SLATE_COLOR
		const FSlateColor GetSlateColor(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FSlateColor& InDefaultValue /* = FStyleDefaults::GetSlateColor() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;
#endif
		const FMargin& GetMargin(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FMargin& InDefaultValue /* = FStyleDefaults::GetMargin() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const override;

	protected:
		const FSlateWidgetStyle* GetWidgetStyleInternal(const FName InDesiredTypeName, const FName InStyleName, const FSlateWidgetStyle* InDefaultStyle, bool bInWarnIfNotFound) const override;

	private:
		
		void OnThemeChangedAndUpdated(const FGuid& InThemeId);
		static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

		TSharedPtr<FStarshipOverrideStyleRawPtrData> WidgetStylePropertyObjectData;

		TMap<FName, FSlateWidgetStyle*> WidgetStyleMap;
		TMap<FName, FSlateBrush*> BrushMap;
		TMap<FName, float*> FloatMap;
		TMap<FName, FVector2D*> Vector2DMap;
		TMap<FName, FLinearColor*> LinearColorMap;
		TMap<FName, FSlateColor*> SlateColorMap;
		TMap<FName, FMargin*> MarginMap;

		FDelegateHandle OnThemeChangedHandle;
		FDelegateHandle OnThemeUpdatedHandle;
	};

private:
	static TSharedPtr<FStyle> StyleInstance;
	static FName StyleSetName;
};