// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StarshipEditorStyleOverriderDefines.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

class SStarshipOverrideStyleEditor;

class SSearchBox;
class FTextFilterExpressionEvaluator;

class SStarshipEditorStyleWidgetTree : public SCompoundWidget
{
public:
	class FStyleTreeItem
	{
	public:

		FStyleTreeItem(TSharedPtr<FStyleTreeItem> InParent,  const FName& InDisplayName, EStarshipOverrideHandlerCategory InCategoryType)
			: Parent(InParent)
			, DisplayName(InDisplayName)
			, CategoryType(InCategoryType)
			, bIsVisible(true)
		{

		}

		bool QueryWidgetTreeItem(const TFunctionRef<bool(const FStyleTreeItem*)>& InQueryFunc) const;

		TWeakPtr<FStyleTreeItem> Parent;
		FName DisplayName;

		TArray<TSharedPtr<FStyleTreeItem>> Children;
		EStarshipOverrideHandlerCategory CategoryType;
		bool bIsVisible;
	};
public:
	SLATE_BEGIN_ARGS(SStarshipEditorStyleWidgetTree)
		{

		}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<SStarshipOverrideStyleEditor> InOwner);

	void QueryWidgetTreeItem(const TFunctionRef<bool(const FStyleTreeItem*)>& InQueryFunc) const;

private:
	void CreateSlateWidgetTypes();
	bool RefreshFilter(FStyleTreeItem& InNodeItem);

	TSharedRef<ITableRow> OnGenerateWidgetForTreeView(TSharedPtr<FStyleTreeItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnGetChildrenForTreeView(TSharedPtr<FStyleTreeItem> InItem, TArray<TSharedPtr<FStyleTreeItem>>& OutChildren);
	void OnSelectionChanged(TSharedPtr<FStyleTreeItem> InItem, ESelectInfo::Type InSelectInfo);
	void OnFilterTextChanged(const FText& InFilterText);

	TSharedPtr<FStyleTreeItem>& GetRoot(EStarshipOverrideHandlerCategory InCategoryType)
	{
		return RootList[static_cast<uint32>(InCategoryType)];
	}

	const TSharedPtr<FStyleTreeItem>& GetRoot(EStarshipOverrideHandlerCategory InCategoryType) const
	{
		return RootList[static_cast<uint32>(InCategoryType)];
	}

	TWeakPtr<SStarshipOverrideStyleEditor> OwnerWeak;
	TMap<FName, TWeakObjectPtr<UScriptStruct>> SlateWidgetStyleList;

	TArray<TSharedPtr<FStyleTreeItem>> SlateDisplayNameList;
	TArray<TSharedPtr<FStyleTreeItem>> TreeViewSourceItemList;

	TStaticArray<TSharedPtr<FStyleTreeItem>, static_cast<uint32>(EStarshipOverrideHandlerCategory::Count)> RootList;

	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<STreeView<TSharedPtr<FStyleTreeItem> > > TreeView;

	TSharedPtr<FTextFilterExpressionEvaluator> TextFilter;

	TMap<FName, TSharedPtr<FStyleTreeItem>> CategoryWidgetMap;

	FText FilterText;
};