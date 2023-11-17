// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StarshipEditorStyleOverriderDefines.h"

struct FSlateWidgetStyle;

struct FStarshipOverrideStyleDefines;

class FStarshipOverrideStyleObjectData;
class FStarshipOverrideStyleRawDataBaseHandler;

class IPropertyHandle;

class FStarshipOverrideStyleRawPtrData
{
public:
	struct ConstructionData
	{
		const FStarshipOverrideStyleDefines& Defines;
		const TMap<FName, const FSlateWidgetStyle*>& DefaultWidgetStyleValueMap;
		const TMap<FName, const FSlateBrush*>& DefaultBrushStyleValueMap;
		const TMap<FName, float>& DefaultFloatStyleValueMap;
		const TMap<FName, FVector2D>& DefaultVectorStyleValueMap;
		const TMap<FName, const FLinearColor*>& DefaultLinearColorValueMap;
		const TMap<FName, FSlateColor>& DefaultSlateColorValueMap;
		const TMap<FName, const FMargin*>& DefaultMarginValueMap;
	};

	FStarshipOverrideStyleRawPtrData();
	~FStarshipOverrideStyleRawPtrData();

	void Create(const ConstructionData& InConstructionData);
	void Create(const FStarshipOverrideStyleRawPtrData& InSrcData);
	void CopyData(const FStarshipOverrideStyleRawPtrData& InSrcData);
	void CopyFromUObject(const UObject& InSrcObject);

	void UnlinkColors();

	void Reset();

	const FStarshipOverrideStyleRawDataBaseHandler* FindHandler(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const;
	void QueryHandler(const TFunctionRef<bool(EStarshipOverrideHandlerCategory , const FName&, const TSharedRef<const FStarshipOverrideStyleRawDataBaseHandler>&)>& InQueryFunc) const;

	TSharedRef<FJsonObject> ToJsonObject() const;
	void SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject);

	bool IsEqualValueFromPropertyHandle(const TSharedRef<IPropertyHandle>& InPropertyHandle, const void* InSrcPtr) const;

	const void* GetRawPtrFromValuePtr(const TSharedRef<IPropertyHandle>& InPropertyHandle) const;

private:
	TStaticArray<TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>, (uint32)EStarshipOverrideHandlerCategory::Count> DataHandlerList;
};

class FStarshipOverrideStyleObjectData
{
public:
	struct FRootClassObjectController
	{
		~FRootClassObjectController()
		{
			for (TObjectPtr<UClass>& ClassObject : RootClassObjectList)
			{
				ClassObject->RemoveFromRoot();
				ClassObject->ConditionalBeginDestroy();
			}
		}

		TArray<TObjectPtr<UClass>> RootClassObjectList;
	};

	FStarshipOverrideStyleObjectData();
	~FStarshipOverrideStyleObjectData();

#if 0
	void CreateUObjects(const FStarshipOverrideStyleRawPtrData& InData, bool bInShouldAddStyleNameMetaData = false);
	void DuplicateUObjects(const FStarshipOverrideStyleObjectData& InData);
#endif

	void AddSpecificUObject(const FStarshipOverrideStyleRawDataBaseHandler& InHandler, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, bool bInShouldAddStyleNameMetaData);
	void AddCopySpecificUobject(const UObject& InSrcObject);
	
	void CopyDataFromProperty(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, const FStarshipOverrideStyleRawPtrData& InSrcData);
	void UnlinkColors();
	void UnlinkColors(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName);

	UObject* FindObject(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const;
	void QueryObject(const TFunctionRef<bool(const FName&, UObject&)>& InQueryFunc) const;

	void Reset();

	TSharedRef<FJsonObject> ToJsonObject() const;

	TSharedPtr<FRootClassObjectController> GetRootClassObjectController();

private:

	void AddSpecificUObjectImpl(const FStarshipOverrideStyleRawDataBaseHandler& InHandler, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, bool bInShouldAddStyleNameMetaData);
	void UnlinkColorsImpl(UObject& InOutObject);

	TStaticArray<TMap<FName, TObjectPtr<UObject>>, static_cast<uint32>(EStarshipOverrideHandlerCategory::Count)> ObjectList;
	TSharedPtr<FRootClassObjectController> RootClassObjectController;
};
