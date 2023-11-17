// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipOverrideStyleRawDataHandler.h"

/* FStarshipOverrideStyleRawDataBaseHandler
 *****************************************************************************/
void FStarshipOverrideStyleRawDataBaseHandler::UnlinkStructProperty(FStructProperty* InStructProperty, void* InContainerPtr)
{
	if (InStructProperty->Struct == FSlateColor::StaticStruct())
	{
		FSlateColor* SlateColor = (FSlateColor*)InContainerPtr;
		SlateColor->Unlink();
		return;
	}
	//else if (InStructProperty->Struct == FSlateBrush::StaticStruct())
	//{
	//	FSlateBrush* SlateColor = (FSlateBrush*)InContainerPtr;
	//	SlateColor->UnlinkColors();
	//	return;
	//}

	for (TFieldIterator<FProperty> It(InStructProperty->Struct); It; ++It)
	{
		FProperty* Property = *It;
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			void* ContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(InContainerPtr);

			// If ScriptStruct is compared in a for loop, the Unlink function cannot be called via FStarshipOverrideStyleRawPtrData.
			//if (StructProperty->Struct == FSlateColor::StaticStruct())
			//{
			//	FSlateColor* SlateColor = (FSlateColor*)ContainerPtr;
			//	SlateColor->Unlink();
			//}
			//else if (StructProperty->Struct == FSlateBrush::StaticStruct())
			//{
			//	FSlateBrush* SlateColor = (FSlateBrush*)ContainerPtr;
			//	SlateColor->UnlinkColors();
			//}
			//else
			{
				UnlinkStructProperty(StructProperty, ContainerPtr);
			}
		}
	}
}

TSharedPtr<FJsonValue> FStarshipOverrideStyleRawDataBaseHandler::ExportColorDataForToJsonObject(FProperty* InProperty, const void* InValue)
{
	if (FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		if (StructProperty->Struct == FSlateColor::StaticStruct())
		{
			UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();
			// Intentionally exclude the JSON Object wrapper, which specifically needs to export JSON in an object representation instead of a string
			if (TheCppStructOps && TheCppStructOps->HasExportTextItem())
			{
				check(false);
				FString OutValueStr;
				TheCppStructOps->ExportTextItem(OutValueStr, InValue, nullptr, nullptr, PPF_None, nullptr);
				return MakeShared<FJsonValueString>(OutValueStr);
			}

			TSharedRef<FJsonObject> Out = MakeShared<FJsonObject>();
			if (FJsonObjectConverter::UStructToJsonObject(StructProperty->Struct, InValue, Out, 0 & (~CPF_ParmFlags), 0, nullptr))
			{
				Out->SetBoolField("StarshipOverride_ColorData", true);
				return MakeShared<FJsonValueObject>(Out);
			}
		}
	}
	return nullptr;
}

/* FStarshipOverrideStyleRawDataStructHandler
 *****************************************************************************/
FStarshipOverrideStyleRawDataStructHandler::FStarshipOverrideStyleRawDataStructHandler(const TWeakObjectPtr<UScriptStruct>& InScriptStruct)
	: ScriptStructType(InScriptStruct)
	, RawData(nullptr)

{
	RawData = (void*)FMemory::Malloc(ScriptStructType->GetStructureSize());
	ScriptStructType->InitializeStruct(RawData);
}

FStarshipOverrideStyleRawDataStructHandler::~FStarshipOverrideStyleRawDataStructHandler()
{
	if (UScriptStruct* ScriptStruct = ScriptStructType.Get())
	{
		ScriptStruct->DestroyStruct(RawData);
	}

	FMemory::Free(RawData);
}

FProperty* FStarshipOverrideStyleRawDataStructHandler::CreateProperty(UObject& InField, const FName& InStyleSetName) const
{
	FStructProperty* Prop = new FStructProperty(&InField, InStyleSetName, EObjectFlags::RF_Transient);
	Prop->SetPropertyFlags(EPropertyFlags::CPF_Edit);
	Prop->Struct = ScriptStructType.Get();

	return Prop;
}

void FStarshipOverrideStyleRawDataStructHandler::SetValue(const void* InSrcData)
{
	check(ScriptStructType.Get());

	ScriptStructType->CopyScriptStruct(RawData, InSrcData);
}

void FStarshipOverrideStyleRawDataStructHandler::SetValueFromUObject(const UObject& InSrcObject)
{
	for (TFieldIterator<FProperty> It(InSrcObject.GetClass()); It; ++It)
	{
		FProperty* Property = *It;
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == ScriptStructType)
			{
				const void* Src = StructProperty->ContainerPtrToValuePtr<void>(&InSrcObject);
				SetValue(Src);
				return;
			}
		}
	}
	check(false);
}

void FStarshipOverrideStyleRawDataStructHandler::UnlinkColros()
{
	if (ScriptStructType == FSlateColor::StaticStruct())
	{
		FSlateColor* SlateColor = (FSlateColor*)RawData;
		SlateColor->Unlink();
		return;
	}
	for (TFieldIterator<FStructProperty> It(ScriptStructType.Get()); It; ++It)
	{
		FStructProperty* StructProperty = *It;
		void* ContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(RawData);
		FStarshipOverrideStyleRawDataBaseHandler::UnlinkStructProperty(StructProperty, ContainerPtr);
	}
}

bool FStarshipOverrideStyleRawDataStructHandler::ToJsonObject(TSharedRef<FJsonObject>& OutJsonObject) const
{
	FJsonObjectConverter::CustomExportCallback ExportCB = FJsonObjectConverter::CustomExportCallback::CreateStatic(&FStarshipOverrideStyleRawDataBaseHandler::ExportColorDataForToJsonObject);
	if (FJsonObjectConverter::UStructToJsonObject(ScriptStructType.Get(), RawData, OutJsonObject, 0, 0, &ExportCB))
	{
		if (ScriptStructType == FSlateColor::StaticStruct())
		{
			OutJsonObject->SetBoolField("StarshipOverride_ColorData", true);
		}

		return true;
	}

	return false;
}

void FStarshipOverrideStyleRawDataStructHandler::SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject)
{
	FJsonObjectConverter::JsonObjectToUStruct(InJsonObject, ScriptStructType.Get(), RawData);
}

TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> FStarshipOverrideStyleRawDataStructHandler::Clone() const
{
	TSharedPtr<FStarshipOverrideStyleRawDataStructHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataStructHandler(ScriptStructType));
	Handler->SetValue(RawData);
	return Handler;
}

