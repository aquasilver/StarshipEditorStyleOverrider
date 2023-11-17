// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "JsonObjectConverter.h"


class FJsonObject;

class FStarshipOverrideStyleRawDataBaseHandler : public TSharedFromThis<FStarshipOverrideStyleRawDataBaseHandler>
{
public:
	FStarshipOverrideStyleRawDataBaseHandler()
	{

	}
	virtual ~FStarshipOverrideStyleRawDataBaseHandler() = default;

	virtual void* GetRawData() = 0;
	virtual const void* GetRawData() const = 0;
	virtual FProperty* CreateProperty(UObject& InField, const FName& InStyleSetName) const = 0;

	virtual void SetValue(const void* InSrcData) = 0;
	virtual void SetValueFromUObject(const UObject& InSrcObject) = 0;

	virtual void UnlinkColros() {}

	virtual bool ToJsonObject(TSharedRef<FJsonObject>& OutJsonObject) const = 0;
	virtual void SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject) = 0;

	virtual TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Clone() const = 0;

	static void UnlinkStructProperty(FStructProperty* InStructProperty, void* InContainerPtr);
	static TSharedPtr<FJsonValue> ExportColorDataForToJsonObject(FProperty* InProperty, const void* InValue);
};

template<typename Type, typename PropertyType>
class FStarshipOverrideStyleRawDataNumericHandler : public FStarshipOverrideStyleRawDataBaseHandler
{
public:
	FStarshipOverrideStyleRawDataNumericHandler()
		: RawData(Type{})
	{

	}

	void* GetRawData() override final { return &RawData; }
	const void* GetRawData() const override final { return &RawData; }
	FProperty* CreateProperty(UObject& InField, const FName& InStyleSetName) const override final
	{
		PropertyType* Prop = new PropertyType(&InField, InStyleSetName, EObjectFlags::RF_Transient); // RF_Public?
		Prop->SetPropertyFlags(EPropertyFlags::CPF_Edit);

		return Prop;
	}

	void SetValue(const void* InSrcData) override
	{
		RawData = *(Type*)InSrcData;
	}

	void SetValueFromUObject(const UObject& InSrcObject) override
	{
		for (TFieldIterator<PropertyType> It(InSrcObject.GetClass()); It; ++It)
		{
			FProperty* Property = *It;
			if (PropertyType* NumericProperty = CastField<PropertyType>(Property))
			{
				const void* Src = NumericProperty->ContainerPtrToValuePtr<void>(&InSrcObject);
				SetValue(Src);
				return;
			}
		}
	}

	bool ToJsonObject(TSharedRef<FJsonObject>& OutJsonObject) const override final
	{
		PropertyType* Property = new PropertyType(FFieldVariant(), NAME_None, EObjectFlags::RF_Transient);

		TSharedPtr<FJsonValue> WriteValue = FJsonObjectConverter::UPropertyToJsonValue(Property, GetRawData());
		if (WriteValue)
		{
			OutJsonObject->SetField("NumericValue", WriteValue);
		}

		delete Property;
		return true;
	}
	void SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject) override final
	{
		TSharedPtr<FJsonValue> ValueProperty = InJsonObject->TryGetField("NumericValue");

		if (ValueProperty)
		{
			PropertyType* Property = new PropertyType(FFieldVariant(), NAME_None, EObjectFlags::RF_Transient);
			FJsonObjectConverter::JsonValueToUProperty(ValueProperty, Property, GetRawData());

			delete Property;
		}
		else
		{
			RawData = Type{};
		}
	}

	TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Clone() const override
	{
		TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Handler = MakeShareable(new FStarshipOverrideStyleRawDataNumericHandler<Type, PropertyType>());
		Handler->SetValue(GetRawData());
		return Handler;
	}

	Type RawData;
};

class FStarshipOverrideStyleRawDataStructHandler : public FStarshipOverrideStyleRawDataBaseHandler
{
public:
	FStarshipOverrideStyleRawDataStructHandler(const TWeakObjectPtr<UScriptStruct>& InScriptStruct);
	~FStarshipOverrideStyleRawDataStructHandler();

	void* GetRawData() override final { return RawData; }
	const void* GetRawData() const override final { return RawData; }
	FProperty* CreateProperty(UObject& InField, const FName& InStyleSetName) const override final;

	void SetValue(const void* InSrcData) override;
	void SetValueFromUObject(const UObject& InSrcObject) override;

	void UnlinkColros() override;

	bool ToJsonObject(TSharedRef<FJsonObject>& OutJsonObject) const override;
	void SetJsonObject(const TSharedRef<FJsonObject>& InJsonObject) override;

	TSharedPtr<FStarshipOverrideStyleRawDataBaseHandler> Clone() const override;

	TWeakObjectPtr<UScriptStruct> ScriptStructType;
	void* RawData;

};
