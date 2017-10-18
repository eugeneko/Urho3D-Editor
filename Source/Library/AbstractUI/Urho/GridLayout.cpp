#include "GridLayout.h"
#include <Urho3D/UI/UIEvents.h>

namespace Urho3D
{

GridLayout::GridLayout(Context* context)
    : UIElement(context)
    , groupData_(1)
{
    SubscribeToEvent(this, E_LAYOUTUPDATED, URHO3D_HANDLER(GridLayout, HandleLayoutUpdated));
}

void GridLayout::InsertItem(unsigned row, unsigned column, UIElement* element, unsigned minWidth, unsigned minHeight)
{
    if (UIElement* oldElement = GetElement(row, column))
        RemoveChild(oldElement);
    EnsureCell(row, column);
    rows_[row].cells_[column].element_ = element;
    rows_[row].cells_[column].minWidth_ = static_cast<int>(minWidth);
    rows_[row].cells_[column].minHeight_ = static_cast<int>(minHeight);
    AddChild(element);
}

void GridLayout::SetRowGroup(unsigned row, unsigned group)
{
    EnsureRow(row);
    rows_[row].group_ = group;
    if (groupData_.Size() <= group)
        groupData_.Resize(group + 1);
}

UIElement* GridLayout::GetElement(unsigned row, unsigned column) const
{
    if (row >= rows_.Size())
        return nullptr;
    if (column >= rows_[row].cells_.Size())
        return nullptr;
    return rows_[row].cells_[column].element_;
}

void GridLayout::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    UpdateChildrenLayout();
}

void GridLayout::UpdateChildrenLayout()
{
    if (childrenLayoutNestingLevel_ > 0)
        return;

    // Disable layout update
    DisableLayoutUpdate();
    ++childrenLayoutNestingLevel_;

    // Update min row heights
    minRowHeights_.Clear();
    minRowHeights_.Resize(rows_.Size(), 0);

    // Gather limits
    for (unsigned groupIndex = 0; groupIndex < groupData_.Size(); ++groupIndex)
    {
        GridRowGroup& group = groupData_[groupIndex];
        group.numRows_ = 0;

        group.minColumnWidth_.Clear();
        group.maxColumnWidth_.Clear();

        for (unsigned rowIndex = 0; rowIndex < rows_.Size(); ++rowIndex)
        {
            const GridRow& row = rows_[rowIndex];
            if (row.group_ != groupIndex)
                continue;

            // Ensure that there is enough columns
            const unsigned numColumns = row.cells_.Size();
            if (group.minColumnWidth_.Size() <= numColumns)
                group.minColumnWidth_.Resize(numColumns, 0);
            if (group.maxColumnWidth_.Size() <= numColumns)
                group.maxColumnWidth_.Resize(numColumns, 0);

            // Calculate sizes
            for (unsigned columnIndex = 0; columnIndex < numColumns; ++columnIndex)
            {
                const GridCell& cell = row.cells_[columnIndex];
                UIElement* cellElement = cell.element_;
                if (!cellElement)
                    continue;

                const IntVector2 minSize = cellElement->GetEffectiveMinSize();
                const IntVector2 maxSize = cellElement->GetMaxSize();

                group.minColumnWidth_[columnIndex] = Max(cell.minWidth_, Max(group.minColumnWidth_[columnIndex], minSize.x_));
                group.maxColumnWidth_[columnIndex] = Max(group.maxColumnWidth_[columnIndex], maxSize.x_);
                minRowHeights_[rowIndex] = Max(cell.minHeight_, Max(minRowHeights_[rowIndex], minSize.y_));
            }
        }

        group.minWidth_ = 0;
        for (int minColumnWidth : group.minColumnWidth_)
            group.minWidth_ += minColumnWidth;
    }

    // Compute min layout size
    minWidth_ = 0;
    minHeight_ = 0;
    for (unsigned groupIndex = 0; groupIndex < groupData_.Size(); ++groupIndex)
    {
        GridRowGroup& group = groupData_[groupIndex];
        minWidth_ = Max(minWidth_, group.minWidth_);
    }
    for (unsigned rowIndex = 0; rowIndex < rows_.Size(); ++rowIndex)
        minHeight_ += minRowHeights_[rowIndex];

    // Resize self
    SetMinSize(minWidth_, minHeight_);
    SetMaxHeight(minHeight_);

    // Compute actual sizes
    const int width = Max(minWidth_, GetWidth());
    for (unsigned groupIndex = 0; groupIndex < groupData_.Size(); ++groupIndex)
    {
        GridRowGroup& group = groupData_[groupIndex];
        const unsigned numColumns = group.minColumnWidth_.Size();

        // Compute horizontal stretch
        group.columnWidth_ = group.minColumnWidth_;
        int remainingStretch = Max(0, width - group.minWidth_);
        unsigned remainingColumns = numColumns;
        while (remainingStretch > 0 && remainingColumns > 0)
        {
            // Count stretchable columns
            remainingColumns = 0;
            for (unsigned i = 0; i < numColumns; ++i)
                if (group.columnWidth_[i] < group.maxColumnWidth_[i])
                    ++remainingColumns;
            if (remainingColumns == 0)
                break;

            // Stretch remaining columns
            const int delta = (remainingStretch + remainingColumns - 1) / remainingColumns;
            for (unsigned i = 0; i < numColumns; ++i)
            {
                if (group.columnWidth_[i] < group.maxColumnWidth_[i])
                {
                    const int oldWidth = group.columnWidth_[i];
                    group.columnWidth_[i] = Min(group.maxColumnWidth_[i], group.columnWidth_[i] + Min(delta, remainingStretch));
                    remainingStretch -= group.columnWidth_[i] - oldWidth;
                }
            }
        }
    }

    // Apply layout
    int y = 0;
    for (unsigned rowIndex = 0; rowIndex < rows_.Size(); ++rowIndex)
    {
        const GridRow& row = rows_[rowIndex];
        const GridRowGroup& group = groupData_[row.group_];
        const int rowHeight = minRowHeights_[rowIndex];

        // Iterate over columns
        int x = 0;
        for (unsigned columnIndex = 0; columnIndex < row.cells_.Size(); ++columnIndex)
        {
            const int columnWidth = group.columnWidth_[columnIndex];
            const GridCell& cell = row.cells_[columnIndex];
            if (UIElement* cellElement = cell.element_)
            {
                const IntVector2 origin(x, y);
                const IntVector2 cellSize(columnWidth, rowHeight);
                const IntVector2 elementSize = VectorMin(cellSize, cellElement->GetMaxSize());

                cellElement->SetPosition(origin);
                cellElement->SetSize(elementSize);
            }
            x += columnWidth;
        }
        y += rowHeight;
    }

    --childrenLayoutNestingLevel_;
    EnableLayoutUpdate();
}

void GridLayout::EnsureRow(unsigned row)
{
    if (rows_.Size() <= row)
        rows_.Resize(row + 1);
}

void GridLayout::EnsureCell(unsigned row, unsigned column)
{
    EnsureRow(row);
    if (rows_[row].cells_.Size() <= column)
        rows_[row].cells_.Resize(column + 1);
}

void GridLayout::HandleLayoutUpdated(StringHash eventType, VariantMap& eventData)
{
    UpdateChildrenLayout();
}

}
