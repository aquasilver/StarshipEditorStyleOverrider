// Copyright 2023 aquasilver, Inc. All Rights Reserved.

#include "StarshipEditorStyleWidgetTree.h"
#include "StarshipOverrideStyleCustomization.h"
#include "StarshipOverrideThemeManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Misc/TextFilterExpressionEvaluator.h"


/* SStarshipEditorStyleWidgetTree - FStyleTreeItem
 *****************************************************************************/
bool SStarshipEditorStyleWidgetTree::FStyleTreeItem::QueryWidgetTreeItem(const TFunctionRef<bool(const FStyleTreeItem*)>& InQueryFunc) const
{
	if (InQueryFunc(this))
	{
		return true;
	}

	for (const TSharedPtr<FStyleTreeItem>& TreeItem : Children)
	{
		if (TreeItem->QueryWidgetTreeItem(InQueryFunc))
		{
			return true;
		}
	}

	return false;
}

/* SStarshipEditorStyleWidgetTree
 *****************************************************************************/
void SStarshipEditorStyleWidgetTree::Construct(const FArguments& InArgs, const TSharedRef<SStarshipOverrideStyleEditor> InOwner)
{
	OwnerWeak = InOwner;

	static const char* RootCategoryDisplayName[] =
	{
		"Widgets",
		"Brushes",
		"Floats",
		"Vector2Ds",
		"LinearColors",
		"SlateColors",
		"Margins"
	};
	static_assert(UE_ARRAY_COUNT(RootCategoryDisplayName) == static_cast<uint32>(EStarshipOverrideHandlerCategory::Count), "");

	for (uint32 i=0; i<static_cast<uint32>(EStarshipOverrideHandlerCategory::Count); i++)
	{
		RootList[i] = MakeShareable(new FStyleTreeItem(nullptr, RootCategoryDisplayName[i], static_cast<EStarshipOverrideHandlerCategory>(i)));
	}

	CreateSlateWidgetTypes();

	TextFilter = MakeShared<FTextFilterExpressionEvaluator>(ETextFilterExpressionEvaluatorMode::BasicString);
	TreeViewSourceItemList = SlateDisplayNameList;

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SAssignNew(SearchBox, SSearchBox)
						.OnTextChanged(this, &SStarshipEditorStyleWidgetTree::OnFilterTextChanged)
				]
				+ SVerticalBox::Slot()
				[
					SAssignNew(TreeView, STreeView<TSharedPtr<FStyleTreeItem>>)
						.ClearSelectionOnClick(false)
						.SelectionMode(ESelectionMode::Single)
						.TreeItemsSource(&TreeViewSourceItemList)
						.OnGenerateRow(this, &SStarshipEditorStyleWidgetTree::OnGenerateWidgetForTreeView)
						.OnGetChildren(this, &SStarshipEditorStyleWidgetTree::OnGetChildrenForTreeView)
						.OnSelectionChanged( this, &SStarshipEditorStyleWidgetTree::OnSelectionChanged )
				]
		];
}

void SStarshipEditorStyleWidgetTree::CreateSlateWidgetTypes()
{
	const FStarshipOverrideStyleDefines& StyleDefines = UStarshipOverrideStyleThemeManager::Get().GetStyleDefines();

	TSharedPtr<FStyleTreeItem>& RootWidget = GetRoot(EStarshipOverrideHandlerCategory::SlateWidgetStyle);
	TSharedPtr<FStyleTreeItem>& RootBrush = GetRoot(EStarshipOverrideHandlerCategory::Brush);
	TSharedPtr<FStyleTreeItem>& RootFloat = GetRoot(EStarshipOverrideHandlerCategory::Float);
	TSharedPtr<FStyleTreeItem>& RootVector2D = GetRoot(EStarshipOverrideHandlerCategory::Vector2D);
	TSharedPtr<FStyleTreeItem>& RootLinearColor = GetRoot(EStarshipOverrideHandlerCategory::LinearColor);
	TSharedPtr<FStyleTreeItem>& RootSlateColor = GetRoot(EStarshipOverrideHandlerCategory::SlateColor);
	TSharedPtr<FStyleTreeItem>& RootMargin = GetRoot(EStarshipOverrideHandlerCategory::Margin);


	for (const FStarshipOverrideStyleDefines::WidgetDefine& DefineData : StyleDefines.StyleWidgetDefineList)
	{
		TSharedPtr<FStyleTreeItem> FoundCategory = CategoryWidgetMap.FindRef(DefineData.DesiredTypeName);
		if (!FoundCategory)
		{
			FoundCategory = MakeShareable(new FStyleTreeItem(RootWidget, DefineData.DesiredTypeName, EStarshipOverrideHandlerCategory::SlateWidgetStyle));
			RootWidget->Children.Add(FoundCategory);
			
			CategoryWidgetMap.Add(DefineData.DesiredTypeName, FoundCategory);
		}

		FoundCategory->Children.Add(MakeShareable(new FStyleTreeItem(RootWidget, DefineData.StyleSetName, EStarshipOverrideHandlerCategory::SlateWidgetStyle)));
	}

	auto CreateSimpleTreeData = [](TSharedPtr<FStyleTreeItem>& OutRoot, const TArray<FName>& InDefineList)
	{
		for (const FName& StyleSetName : InDefineList)
		{
			OutRoot->Children.Add(MakeShareable(new FStyleTreeItem(OutRoot, StyleSetName, OutRoot->CategoryType)));
		}
	};

	CreateSimpleTreeData(RootFloat, StyleDefines.FloatDefineList);
	CreateSimpleTreeData(RootBrush, StyleDefines.BrushDefineList);
	CreateSimpleTreeData(RootVector2D, StyleDefines.Vector2DDefineList);
	CreateSimpleTreeData(RootLinearColor, StyleDefines.LinearColorDefineList);
	CreateSimpleTreeData(RootSlateColor, StyleDefines.SlateColorDefineList);
	CreateSimpleTreeData(RootMargin, StyleDefines.MarginDefineList);

	for (TSharedPtr<FStyleTreeItem>& Root : RootList)
	{
		if (Root->Children.Num() > 0)
		{
			SlateDisplayNameList.Add(Root);
		}
	}
}

TSharedRef<ITableRow> SStarshipEditorStyleWidgetTree::OnGenerateWidgetForTreeView(TSharedPtr<FStyleTreeItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	return SNew(STableRow< TSharedPtr<FStyleTreeItem> >, InOwnerTable)
		[
			SNew(STextBlock).Text(FText::FromName(InItem->DisplayName))
		];
}

bool SStarshipEditorStyleWidgetTree::RefreshFilter(FStyleTreeItem& InNodeItem)
{
	InNodeItem.bIsVisible = false;

	for (TSharedPtr<FStyleTreeItem>& ChildNodeItem : InNodeItem.Children)
	{
		bool bIsVisible = RefreshFilter(*ChildNodeItem);

		InNodeItem.bIsVisible |= bIsVisible;
	}

	if (!InNodeItem.bIsVisible)
	{
		InNodeItem.bIsVisible = FilterText.IsEmpty() || TextFilter->TestTextFilter(FBasicStringFilterExpressionContext(InNodeItem.DisplayName.ToString()));
	}

	return InNodeItem.bIsVisible;
}

void SStarshipEditorStyleWidgetTree::OnGetChildrenForTreeView(TSharedPtr<FStyleTreeItem> InItem, TArray<TSharedPtr<FStyleTreeItem>>& OutChildren)
{
	for (const TSharedPtr<FStyleTreeItem>& Child : InItem->Children)
	{
		if (Child->bIsVisible)
		{
			OutChildren.Add(Child);
		}
	}
}

void SStarshipEditorStyleWidgetTree::OnSelectionChanged(TSharedPtr<FStyleTreeItem> InItem, ESelectInfo::Type InSelectInfo)
{
	if (InItem)
	{
		OwnerWeak.Pin()->OnSelectionChanged(InItem->DisplayName, InItem->CategoryType);
	}
}

void SStarshipEditorStyleWidgetTree::OnFilterTextChanged(const FText& InFilterText)
{
	FilterText = InFilterText;
	TreeViewSourceItemList.Empty();

	TextFilter->SetFilterText(InFilterText);

	for (TSharedPtr<FStyleTreeItem>& NodeItem : SlateDisplayNameList)
	{
		bool bIsVisible = RefreshFilter(*NodeItem);

		if (bIsVisible)
		{
			TreeViewSourceItemList.Add(NodeItem);

			TreeView->SetItemExpansion(NodeItem, true);
		}
	}

	TreeView->RequestTreeRefresh();
}

void SStarshipEditorStyleWidgetTree::QueryWidgetTreeItem(const TFunctionRef<bool(const FStyleTreeItem*)>& InQueryFunc) const
{
	GetRoot(EStarshipOverrideHandlerCategory::SlateWidgetStyle)->QueryWidgetTreeItem(InQueryFunc);
}
