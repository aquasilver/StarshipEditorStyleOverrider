// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyle.h"
#include "StarshipOverrideThemeManager.h"
#include "StarshipOverrideStyleSettings.h"
#include "StarshipOverrideStyleRawDataHandler.h"
#include "Styling/StyleColors.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"
#include "Interfaces/IPluginManager.h"



TSharedPtr<FStarshipOverrideStyle::FStyle> FStarshipOverrideStyle::StyleInstance = nullptr;
FName FStarshipOverrideStyle::StyleSetName = TEXT("OverrideEditorStyle");


#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FStarshipOverrideStyle::FStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_PLUGIN_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush( FStarshipOverrideStyle::FStyle::InContent( RelativePath, ".svg" ), __VA_ARGS__ )


/* FStarshipOverrideStyle - FStyle
 *****************************************************************************/
FStarshipOverrideStyle::FStyle::FStyle()
	: FSlateStyleSet(FStarshipOverrideStyle::StyleSetName)
{

}


FStarshipOverrideStyle::FStyle::~FStyle()
{
	if (IsClassLoaded<UStarshipOverrideStyleThemeManager>())
	{
		UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();
		ThemeManager.OnThemeChanged().Remove(OnThemeChangedHandle);
		ThemeManager.OnThemeUpdated().Remove(OnThemeUpdatedHandle);
	}
}

void FStarshipOverrideStyle::FStyle::Initialize()
{
	SetParentStyleName("EditorStyle");

	SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	Set("StarshipOverride.HasEditData", new CORE_IMAGE_BRUSH_SVG("Starship/SourceControl/SCC_CheckedOut", CoreStyleConstants::Icon16x16, FStyleColors::Error));

	UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();

	WidgetStylePropertyObjectData = MakeShared<FStarshipOverrideStyleRawPtrData>();
	WidgetStylePropertyObjectData->Create(ThemeManager.GetCurrentTheme().ObjectData);

	WidgetStylePropertyObjectData->QueryHandler([this](EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, const TSharedRef<const FStarshipOverrideStyleRawDataBaseHandler>& InHandler) -> bool
	{

		switch (InCategoryType)
		{
		case EStarshipOverrideHandlerCategory::SlateWidgetStyle:
			WidgetStyleMap.Add(InStyleSetName, (FSlateWidgetStyle*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::Brush:
			BrushMap.Add(InStyleSetName, (FSlateBrush*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::Float:
			FloatMap.Add(InStyleSetName, (float*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::Vector2D:
			Vector2DMap.Add(InStyleSetName, (FVector2D*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::LinearColor:
			LinearColorMap.Add(InStyleSetName, (FLinearColor*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::SlateColor:
			SlateColorMap.Add(InStyleSetName, (FSlateColor*)InHandler->GetRawData());
			break;
		case EStarshipOverrideHandlerCategory::Margin:
			MarginMap.Add(InStyleSetName, (FMargin*)InHandler->GetRawData());
			break;
		default:
			check(false);
		}

		return false;
	});

	OnThemeChangedHandle = ThemeManager.OnThemeChanged().AddRaw(this, &FStarshipOverrideStyle::FStyle::OnThemeChangedAndUpdated);
	OnThemeUpdatedHandle = ThemeManager.OnThemeUpdated().AddRaw(this, &FStarshipOverrideStyle::FStyle::OnThemeChangedAndUpdated);
}


float FStarshipOverrideStyle::FStyle::GetFloat(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, float InDefaultValue /* = FStyleDefaults::GetFloat() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const float* FoundFloat = FloatMap.FindRef(InPropertyName);
	if (FoundFloat)
	{
		return *FoundFloat;
	}
	return FSlateStyleSet::GetFloat(InPropertyName, InSpecifier, InDefaultValue, InRequestingStyle);
}

FVector2D FStarshipOverrideStyle::FStyle::GetVector(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, FVector2D InDefaultValue /* = FStyleDefaults::GetVector2D() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const FVector2D* FoundVector = Vector2DMap.FindRef(InPropertyName);
	if (FoundVector)
	{
		return *FoundVector;
	}

	return FSlateStyleSet::GetVector(InPropertyName, InSpecifier, InDefaultValue, InRequestingStyle);
}

const FSlateBrush* FStarshipOverrideStyle::FStyle::GetBrush(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const FSlateBrush* FoundBrush = BrushMap.FindRef(InPropertyName);
	if (FoundBrush)
	{
		return FoundBrush;
	}

	return FSlateStyleSet::GetBrush(InPropertyName, InSpecifier, InRequestingStyle);
}

const FLinearColor& FStarshipOverrideStyle::FStyle::GetColor(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FLinearColor& InDefaultValue /* = FStyleDefaults::GetColor() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const FLinearColor* FoundColor = LinearColorMap.FindRef(InPropertyName);
	if (FoundColor)
	{
		return *FoundColor;
	}

	return FSlateStyleSet::GetColor(InPropertyName, InSpecifier, InDefaultValue, InRequestingStyle);
}

#if ENABLE_STARSHIP_OVERRIDE_SLATE_COLOR
const FSlateColor FStarshipOverrideStyle::FStyle::GetSlateColor(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FSlateColor& InDefaultValue /* = FStyleDefaults::GetSlateColor() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const FSlateColor* FoundColor = SlateColorMap.FindRef(InPropertyName);
	if (FoundColor)
	{
		return *FoundColor;
	}

	return FSlateStyleSet::GetSlateColor(InPropertyName, InSpecifier, InDefaultValue, InRequestingStyle);
}
#endif

const FMargin& FStarshipOverrideStyle::FStyle::GetMargin(const FName InPropertyName, const ANSICHAR* InSpecifier /* = nullptr */, const FMargin& InDefaultValue /* = FStyleDefaults::GetMargin() */, const ISlateStyle* InRequestingStyle /* = nullptr */) const
{
	const FMargin* FoundMargin = MarginMap.FindRef(InPropertyName);
	if (FoundMargin)
	{
		return *FoundMargin;
	}

	return FSlateStyleSet::GetMargin(InPropertyName, InSpecifier, InDefaultValue, InRequestingStyle);
}

const FSlateWidgetStyle* FStarshipOverrideStyle::FStyle::GetWidgetStyleInternal(const FName InDesiredTypeName, const FName InStyleName, const FSlateWidgetStyle* InDefaultStyle, bool bInWarnIfNotFound) const
{
	const FSlateWidgetStyle* FoundStyle = WidgetStyleMap.FindRef(InStyleName);
	if (FoundStyle)
	{
		return FoundStyle;
	}

	return FSlateStyleSet::GetWidgetStyleInternal(InDesiredTypeName, InStyleName, InDefaultStyle, bInWarnIfNotFound);
	//if (FoundStyle == nullptr)
	//{
	//	if (bWarnIfNotFound)
	//	{
	//		Log(EStyleMessageSeverity::Warning, FText::Format(NSLOCTEXT("SlateStyleSet", "UnknownWidgetStyle", "Unable to find Slate Widget Style '{0}'. Using {1} defaults instead."), FText::FromName(StyleName), FText::FromName(DesiredTypeName)));
	//	}

	//	return DefaultStyle;
	//}

	//if (FoundStyle->GetTypeName() != DesiredTypeName)
	//{
	//	if (bWarnIfNotFound)
	//	{
	//		Log(EStyleMessageSeverity::Error, FText::Format(NSLOCTEXT("SlateStyleSet", "WrongWidgetStyleType", "The Slate Widget Style '{0}' is not of the desired type. Desired: '{1}', Actual: '{2}'"), FText::FromName(StyleName), FText::FromName(DesiredTypeName), FText::FromName(FoundStyle->GetTypeName())));
	//	}

	//	return nullptr;
	//}

	//// Fallback
	//return FSlateStyleSet::GetWidgetStyleInternal(DesiredTypeName, StyleName, DefaultStyle, bWarnIfNotFound);
}


void FStarshipOverrideStyle::FStyle::OnThemeChangedAndUpdated(const FGuid& /*InThemeId*/)
{
	const UStarshipOverrideStyleSettings* Settings = GetMutableDefault<UStarshipOverrideStyleSettings>();
	if (Settings->bIsStyleNotAppliedAfterStartup)
	{
		return;
	}

	UStarshipOverrideStyleThemeManager& ThemeManager = UStarshipOverrideStyleThemeManager::Get();

	WidgetStylePropertyObjectData->CopyData(ThemeManager.GetCurrentTheme().ObjectData);
}


FString FStarshipOverrideStyle::FStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("StarshipEditorStyleOverrider"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

/* FStarshipOverrideStyle
 *****************************************************************************/
void FStarshipOverrideStyle::Initialize()
{
	StyleInstance = MakeShareable(new FStyle());
	StyleInstance->Initialize();

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance.Get());
}

void FStarshipOverrideStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
	StyleInstance.Reset();
}

const FName& FStarshipOverrideStyle::GetStyleSetName()
{
	return StyleSetName;
}

TSharedRef<FStarshipOverrideStyleRawPtrData> FStarshipOverrideStyle::GetFrontlineSlateStyleObject()
{
	return StyleInstance->GetStyleObjectData();
}
