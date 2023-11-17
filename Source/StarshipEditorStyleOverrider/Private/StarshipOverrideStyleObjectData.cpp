// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyleObjectData.h"
#include "JsonObjectConverter.h"
#include "StarshipOverrideThemeManager.h"
#include "StarshipOverridePropertyCustomization.h"
#include "StarshipOverrideStyleRawDataHandler.h"

namespace PrivateCPP
{
	const FString& ConvertEHandlerCategoryToString(EStarshipOverrideHandlerCategory InCategoryType)
	{
		static const FString Table[] =
		{
			TEXT("SlateWidgetStyle_Category"),
			TEXT("Brush_Category"),
			TEXT("Float_Category"),
			TEXT("Vector2D_Category"),
			TEXT("LinearColor_Category"),
			TEXT("SlateColor_Category"),
			TEXT("Margin_Category")
		};

		static_assert(UE_ARRAY_COUNT(Table) == static_cast<uint32>(EStarshipOverrideHandlerCategory::Count), "");

		return Table[static_cast<uint32>(InCategoryType)];
	}
}


/* FStarshipOverrideStyleRawPtrData
 *****************************************************************************/
FStarshipOverrideStyleRawPtrData::FStarshipOverrideStyleRawPtrData()
{

}

FStarshipOverrideStyleRawPtrData::~FStarshipOverrideStyleRawPtrData()
{
	Reset();
}

void FStarshipOverrideStyleRawPtrData::Create(const ConstructionData& InConstructionData)
{
	Reset();

	for (const FStarshipOverrideStyleDefines::WidgetDefine& DefineData : InConstructionData.Defines.StyleWidgetDefineList)
	{
		TSharedPtr<FStarshipOverrideStyleRawDataStructHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataStructHandler(DefineData.SlateWidgetType));
		DataHandlerList[static_cast<uint32>(EStarshipOverrideHandlerCategory::SlateWidgetStyle)].Add(DefineData.StyleSetName, Handler);
		Handler->SetValue(InConstructionData.DefaultWidgetStyleValueMap.FindChecked(DefineData.StyleSetName));
	}

	for (const FName& StyleSetName : InConstructionData.Defines.FloatDefineList)
	{
		TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataNumericHandler<float, FFloatProperty>());
		DataHandlerList[static_cast<uint32>(EStarshipOverrideHandlerCategory::Float)].Add(StyleSetName, Handler);
		Handler->SetValue(&InConstructionData.DefaultFloatStyleValueMap.FindChecked(StyleSetName));
	}

	auto CreateSimpleStructHandler = [this](UScriptStruct* InStaticStruct, const TArray<FName>& InDefineList, const auto& InDefaultValueMap, EStarshipOverrideHandlerCategory InCategoryType)
	{
		for (const FName& StyleSetName : InDefineList)
		{
			TSharedPtr<FStarshipOverrideStyleRawDataStructHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataStructHandler(InStaticStruct));
			DataHandlerList[static_cast<uint32>(InCategoryType)].Add(StyleSetName, Handler);
			Handler->SetValue(&InDefaultValueMap.FindChecked(StyleSetName));
		}
	};

	auto CreateSimpleStructHandlerForPtr = [this](UScriptStruct* InStaticStruct, const TArray<FName>& InDefineList, const auto& InDefaultValueMap, EStarshipOverrideHandlerCategory InCategoryType)
	{
		for (const FName& StyleSetName : InDefineList)
		{
			TSharedPtr<FStarshipOverrideStyleRawDataStructHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataStructHandler(InStaticStruct));
			DataHandlerList[static_cast<uint32>(InCategoryType)].Add(StyleSetName, Handler);
			Handler->SetValue(InDefaultValueMap.FindChecked(StyleSetName));
		}
	};

	CreateSimpleStructHandlerForPtr(FSlateBrush::StaticStruct(), InConstructionData.Defines.BrushDefineList, InConstructionData.DefaultBrushStyleValueMap, EStarshipOverrideHandlerCategory::Brush);
	CreateSimpleStructHandler(TBaseStructure<FVector2D>::Get(), InConstructionData.Defines.Vector2DDefineList, InConstructionData.DefaultVectorStyleValueMap, EStarshipOverrideHandlerCategory::Vector2D);
	CreateSimpleStructHandlerForPtr(TBaseStructure<FLinearColor>::Get(), InConstructionData.Defines.LinearColorDefineList, InConstructionData.DefaultLinearColorValueMap, EStarshipOverrideHandlerCategory::LinearColor);
	CreateSimpleStructHandler(FSlateColor::StaticStruct(), InConstructionData.Defines.SlateColorDefineList, InConstructionData.DefaultSlateColorValueMap, EStarshipOverrideHandlerCategory::SlateColor);
	CreateSimpleStructHandlerForPtr(FMargin::StaticStruct(), InConstructionData.Defines.MarginDefineList, InConstructionData.DefaultMarginValueMap, EStarshipOverrideHandlerCategory::Margin);
}

void FStarshipOverrideStyleRawPtrData::Create(const FStarshipOverrideStyleRawPtrData& InSrcData)
{
	Reset();

	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap = InSrcData.DataHandlerList[i];
		for (const TPair<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& KeyValue : DataHandlerMap)
		{
			TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Handler = KeyValue.Value->Clone();
			DataHandlerList[i].Add(KeyValue.Key, Handler);
		}
	}
}

void FStarshipOverrideStyleRawPtrData::CopyData(const FStarshipOverrideStyleRawPtrData& InSrcData)
{
	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap = InSrcData.DataHandlerList[i];
		for (const TPair<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& KeyValue : DataHandlerMap)
		{
			//const TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>& Handler = DataHandlerMap.FindChecked(KeyValue.Key);
			TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>& Handler = DataHandlerList[i].FindChecked(KeyValue.Key);
			Handler->SetValue(KeyValue.Value->GetRawData());
		}
	}
}

void FStarshipOverrideStyleRawPtrData::CopyFromUObject(const UObject& InSrcObject)
{
	const FString& StyleSetNameStr = InSrcObject.GetClass()->GetMetaData("StarshipOverride_StyleSetName");
	const FString& CategoryTypeStr = InSrcObject.GetClass()->GetMetaData("StarshipOverride_CategoryType");

	const int CategoryType = FCString::Atoi(*CategoryTypeStr);

	FName StyleSetName(StyleSetNameStr);
	TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>& FoundHandler = DataHandlerList[CategoryType].FindChecked(StyleSetName);
	FoundHandler->SetValueFromUObject(InSrcObject);
}

void FStarshipOverrideStyleRawPtrData::UnlinkColors()
{
	for (const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap : DataHandlerList)
	{
		for (const TPair<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& KeyValue : DataHandlerMap)
		{
			KeyValue.Value->UnlinkColros();
		}
	}
}

void FStarshipOverrideStyleRawPtrData::Reset()
{
	for (TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap : DataHandlerList)
{
		DataHandlerMap.Empty();
	}
}

const FStarshipOverrideStyleRawDataBaseHandler* FStarshipOverrideStyleRawPtrData::FindHandler(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const
{
	const TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>* FoundHandler = DataHandlerList[static_cast<uint32>(InCategoryType)].Find(InStyleSetName);
	if (FoundHandler)
	{
		return (*FoundHandler).Get();
	}

	return nullptr;
}

void FStarshipOverrideStyleRawPtrData::QueryHandler(const TFunctionRef<bool(EStarshipOverrideHandlerCategory, const FName&, const TSharedRef<const FStarshipOverrideStyleRawDataBaseHandler>&)>& InQueryFunc) const
{
	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap = DataHandlerList[i];
		const EStarshipOverrideHandlerCategory CategoryType = static_cast<EStarshipOverrideHandlerCategory>(i);
		for (const TPair<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& KeyValue : DataHandlerMap)
		{
			if (InQueryFunc(CategoryType, KeyValue.Key, KeyValue.Value.ToSharedRef()))
			{
				break;
			}
		}
	}
}

TSharedRef<FJsonObject> FStarshipOverrideStyleRawPtrData::ToJsonObject() const
{
	TSharedRef<FJsonObject> RootJsonObject = MakeShared<FJsonObject>();

	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap = DataHandlerList[i];
		TSharedRef<FJsonObject> CategoryRootJsonObject = MakeShared<FJsonObject>();
		for (const TPair<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& KeyValue : DataHandlerMap)
		{
			/*
			{
				"horizontalBackgroundImage":
				{
					...
				}
			}

			In order to output the same format as FStarshipOverrideStyleObjectData::ToJsonObject, SetObjectField is done after creating Child once.
			*/
			TSharedRef<FJsonObject> ChildJsonObject = MakeShared<FJsonObject>();

			if (KeyValue.Value->ToJsonObject(ChildJsonObject))
			{
				CategoryRootJsonObject->SetObjectField(KeyValue.Key.ToString(), ChildJsonObject.ToSharedPtr());
			}
		}
		if (CategoryRootJsonObject->Values.Num() > 0)
		{
			RootJsonObject->SetObjectField(PrivateCPP::ConvertEHandlerCategoryToString(static_cast<EStarshipOverrideHandlerCategory>(i)), CategoryRootJsonObject);
		}
	}

	return RootJsonObject;
}


void FStarshipOverrideStyleRawPtrData::SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject)
{
	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		const TSharedPtr<FJsonObject>& CategoryRootJsonObject = InJsonObject->GetObjectField(PrivateCPP::ConvertEHandlerCategoryToString(static_cast<EStarshipOverrideHandlerCategory>(i)));
		if (!CategoryRootJsonObject.IsValid())
		{
			continue;
		}
		for (const TPair<FString, TSharedPtr<FJsonValue>>& KeyValue : CategoryRootJsonObject->Values)
		{
			TSharedPtr<FJsonObject> ChildJsonObject = KeyValue.Value->AsObject();
			if (ChildJsonObject)
			{
				TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>* Handler = DataHandlerList[i].Find(FName(KeyValue.Key));
				if (Handler)
				{
					(*Handler)->SetJsonObject(ChildJsonObject.ToSharedRef());
				}
			}
		}
	}
}

bool FStarshipOverrideStyleRawPtrData::IsEqualValueFromPropertyHandle(const TSharedRef<IPropertyHandle>& InPropertyHandle, const void* InSrcPtr) const
{
	const void* ComparePtr = GetRawPtrFromValuePtr(InPropertyHandle);
	return InPropertyHandle->GetProperty()->Identical(ComparePtr, InSrcPtr);
}

const void* FStarshipOverrideStyleRawPtrData::GetRawPtrFromValuePtr(const TSharedRef<IPropertyHandle>& InPropertyHandle) const
{
	if (TSharedPtr<IPropertyHandle> ParentHandle = InPropertyHandle->GetParentHandle())
	{
		if (ParentHandle->GetProperty())
		{
			const void* ValuePtr = GetRawPtrFromValuePtr(ParentHandle.ToSharedRef());

			return InPropertyHandle->GetProperty()->ContainerPtrToValuePtr<void>(ValuePtr);
		}

	}

	//const FWidgetStyleCollection& FoundCollection = StyleRawDataList.FindChecked(InPropertyHandle->GetProperty()->GetFName());

	const FName PropertyName = InPropertyHandle->GetProperty()->GetFName();

	for (const TMap<FName, TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>>& DataHandlerMap : DataHandlerList)
	{
		const TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler>* Handler = DataHandlerMap.Find(PropertyName);
		if (Handler)
		{
			return (*Handler)->GetRawData();
		}
	}

	check(false);
	return nullptr;
}

/* FStarshipOverrideStyleObjectData
 *****************************************************************************/
FStarshipOverrideStyleObjectData::FStarshipOverrideStyleObjectData()
{

}

FStarshipOverrideStyleObjectData::~FStarshipOverrideStyleObjectData()
{
	Reset();
}

#if 0
void FStarshipOverrideStyleObjectData::CreateUObjects(const FStarshipOverrideStyleRawPtrData& InData, bool bInShouldAddStyleNameMetaData)
{
	Reset();

	RootClassObjectController = MakeShareable(new FRootClassObjectController());

	InData.QueryHandler([this, bInShouldAddStyleNameMetaData](EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, const TSharedRef<const FStarshipOverrideStyleRawDataBaseHandler>& InHandler)
	{
		AddSpecificUObjectImpl(InHandler.Get(), InCategoryType, InStyleSetName, bInShouldAddStyleNameMetaData);
		return false;
	});
}


void FStarshipOverrideStyleObjectData::DuplicateUObjects(const FStarshipOverrideStyleObjectData& InData)
{
	Reset();

	for (uint32 i = 0; i < static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		for (const TPair<FName, TObjectPtr<UObject>>& KeyValue : InData.ObjectList[i])
		{
			UObject* DuplicateObj = DuplicateObject<UObject>(KeyValue.Value, nullptr);
			if (!DuplicateObj->IsRooted())
			{
				DuplicateObj->AddToRoot();
			}
			ObjectList[i].Add(KeyValue.Key, DuplicateObj);
		}
	}

	RootClassObjectController = InData.RootClassObjectController;
}
#endif

void FStarshipOverrideStyleObjectData::AddSpecificUObject(const FStarshipOverrideStyleRawDataBaseHandler& InHandler, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, bool bInShouldAddStyleNameMetaData)
{
	AddSpecificUObjectImpl(InHandler, InCategoryType, InStyleSetName, bInShouldAddStyleNameMetaData);
}

void FStarshipOverrideStyleObjectData::AddCopySpecificUobject(const UObject& InSrcObject)
{
	const FString& StyleSetNameStr = InSrcObject.GetClass()->GetMetaData("StarshipOverride_StyleSetName");
	const FString& CategoryTypeStr = InSrcObject.GetClass()->GetMetaData("StarshipOverride_CategoryType");

	const int CategoryType = FCString::Atoi(*CategoryTypeStr);
	FName StyleSetName(StyleSetNameStr);

	UObject* DuplicateObj = DuplicateObject<UObject>(&InSrcObject, nullptr);
	if (!DuplicateObj->IsRooted())
	{
		DuplicateObj->AddToRoot();
	}
	ObjectList[CategoryType].Add(StyleSetName, DuplicateObj);

	//RootClassObjectController->RootClassObjectList.AddUnique(DuplicateObj->GetClass());
}

void FStarshipOverrideStyleObjectData::CopyDataFromProperty(const TSharedRef<IPropertyHandle>& InPropertyHandle, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, const FStarshipOverrideStyleRawPtrData& InSrcData)
{
	const void* CopyPtr = InSrcData.GetRawPtrFromValuePtr(InPropertyHandle);
	void* DestPtr = InPropertyHandle->GetValueBaseAddress((uint8*)FindObject(InCategoryType, InStyleSetName));
	InPropertyHandle->GetProperty()->CopySingleValue(DestPtr, CopyPtr);

	// When using PropertyHandle functions, the type must be specified, so it cannot be used
	{
		//FString StringData;
		//InPropertyHandle->GetProperty()->ExportTextItem_Direct(StringData, CopyPtr, nullptr, nullptr, PPF_None);
		//const TCHAR* ImportTextPtr = *StringData;

		// Maybe ok and need manual notify changed event
		//InPropertyHandle->GetProperty()->ImportText_Direct(ImportTextPtr, DestPtr, nullptr, PPF_None);

		// Not working
		//InPropertyHandle->SetValue(StringData);
	}
}

void FStarshipOverrideStyleObjectData::UnlinkColors()
{
	for (TMap<FName, TObjectPtr<UObject>>& ObjectMap : ObjectList)
	{
		for (const TPair<FName, TObjectPtr<UObject>>& KeyValue : ObjectMap)
		{
			UnlinkColorsImpl(*KeyValue.Value.Get());
		}
	}
}

void FStarshipOverrideStyleObjectData::UnlinkColors(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName)
{
	TObjectPtr<UObject>* FoundObject = ObjectList[static_cast<uint32>(InCategoryType)].Find(InStyleSetName);
	if (FoundObject)
	{
		UnlinkColorsImpl(*(*FoundObject).Get());
	}
}

UObject* FStarshipOverrideStyleObjectData::FindObject(EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName) const
{
	const TObjectPtr<UObject>* FoundObject = ObjectList[static_cast<uint32>(InCategoryType)].Find(InStyleSetName);
	if (FoundObject)
	{
		return *FoundObject;
	}

	return nullptr;
}


void FStarshipOverrideStyleObjectData::QueryObject(const TFunctionRef<bool(const FName&, UObject&)>& InQueryFunc) const
{
	for (const TMap<FName, TObjectPtr<UObject>>& ObjectMap : ObjectList)
	{
		for (const TPair<FName, TObjectPtr<UObject>>& KeyValue : ObjectMap)
		{
			if (InQueryFunc(KeyValue.Key, *KeyValue.Value.Get()))
			{
				return;
			}
		}
	}
}

void FStarshipOverrideStyleObjectData::Reset()
{
	for (TMap<FName, TObjectPtr<UObject>>& ObjectMap : ObjectList)
	{
		for (const TPair<FName, TObjectPtr<UObject>>& DefaultClassObject : ObjectMap)
		{
			DefaultClassObject.Value->RemoveFromRoot();
			DefaultClassObject.Value->ConditionalBeginDestroy();
		}

		ObjectMap.Empty();
	}

	RootClassObjectController.Reset();
}

TSharedRef<FJsonObject> FStarshipOverrideStyleObjectData::ToJsonObject() const
{
	TSharedRef<FJsonObject> RootJsonObject = MakeShared<FJsonObject>();

	FJsonObjectConverter::CustomExportCallback ExportCB = FJsonObjectConverter::CustomExportCallback::CreateStatic(&FStarshipOverrideStyleRawDataBaseHandler::ExportColorDataForToJsonObject);

	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		TSharedRef<FJsonObject> CategoryRootJsonObject = MakeShared<FJsonObject>();
		for (const TPair<FName, TObjectPtr<UObject>>& KeyValue : ObjectList[i])
		{
			/*
			{
				"scrollbar":
				{
					"horizontalBackgroundImage":
					{
						....
					}
				}
			*/
			FJsonObjectConverter::UStructToJsonObject(KeyValue.Value->GetClass(), KeyValue.Value, CategoryRootJsonObject, 0, 0, &ExportCB);
		}

		if (CategoryRootJsonObject->Values.Num() > 0)
		{
			RootJsonObject->SetObjectField(PrivateCPP::ConvertEHandlerCategoryToString(static_cast<EStarshipOverrideHandlerCategory>(i)), CategoryRootJsonObject);
		}
	}

	return RootJsonObject;
}

TSharedPtr<FStarshipOverrideStyleObjectData::FRootClassObjectController> FStarshipOverrideStyleObjectData::GetRootClassObjectController()
{
	if (!RootClassObjectController)
	{
		RootClassObjectController = MakeShareable(new FRootClassObjectController());
	}
	return RootClassObjectController;
}

void FStarshipOverrideStyleObjectData::AddSpecificUObjectImpl(const FStarshipOverrideStyleRawDataBaseHandler& InHandler, EStarshipOverrideHandlerCategory InCategoryType, const FName& InStyleSetName, bool bInShouldAddStyleNameMetaData)
{
	UClass* ObjectData = NewObject<UClass>(GetTransientPackage(), FName(), RF_Transient);
	ObjectData->SetSuperStruct(UObject::StaticClass());

	FProperty* Prop = InHandler.CreateProperty(*ObjectData, InStyleSetName);
	ObjectData->AddCppProperty(Prop);

	ObjectData->Bind();
	ObjectData->StaticLink(true);
	ObjectData->AssembleReferenceTokenStream();
	if (!ObjectData->IsRooted())
	{
		ObjectData->AddToRoot();
	}

	if (bInShouldAddStyleNameMetaData)
	{
		ObjectData->SetMetaData("DisplayName", TEXT("Style Parameters"));
		ObjectData->SetMetaData("StarshipOverride_StyleSetName", *InStyleSetName.ToString());
		ObjectData->SetMetaData("StarshipOverride_CategoryType", *FString::Printf(TEXT("%d"), InCategoryType));
	}
	ObjectData->GetDefaultObject()->AddToRoot();

	UObject* DefaultObject = ObjectData->GetDefaultObject();
	void* Dest = Prop->ContainerPtrToValuePtr<void>(DefaultObject);

	Prop->CopySingleValue(Dest, InHandler.GetRawData());

	ObjectList[static_cast<uint32>(InCategoryType)].Add(InStyleSetName, DefaultObject);
	RootClassObjectController->RootClassObjectList.Add(ObjectData);
}

void FStarshipOverrideStyleObjectData::UnlinkColorsImpl(UObject& InOutObject)
{
	for (TFieldIterator<FStructProperty> It(InOutObject.GetClass()); It; ++It)
	{
		FStructProperty* StructProperty = *It;
		void* ContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(&InOutObject);
		FStarshipOverrideStyleRawDataBaseHandler::UnlinkStructProperty(StructProperty, ContainerPtr);
	}
}

