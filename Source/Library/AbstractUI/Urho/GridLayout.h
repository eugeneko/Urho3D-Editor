#pragma once

#include <Urho3D/UI/UIElement.h>

namespace Urho3D
{

struct GridCell
{
    WeakPtr<UIElement> element_ = nullptr;
    int minWidth_ = 0;
    int minHeight_ = 0;
};

struct GridRow
{
    unsigned group_ = 0;
    Vector<GridCell> cells_;
};

struct GridRowGroup
{
    unsigned numRows_ = 0;
    Vector<int> minColumnWidth_;
    Vector<int> maxColumnWidth_;
    Vector<int> columnWidth_;
    int minWidth_ = 0;
};

class GridLayout : public UIElement
{
    URHO3D_OBJECT(GridLayout, UIElement);

public:
    GridLayout(Context* context);

    void InsertItem(unsigned row, unsigned column, UIElement* element, unsigned minWidth = 0, unsigned minHeight = 0);
    void SetRowGroup(unsigned row, unsigned group);

    UIElement* GetElement(unsigned row, unsigned column) const;

    void OnResize(const IntVector2& newSize, const IntVector2& delta) override;

private:
    void HandleLayoutUpdated(StringHash eventType, VariantMap& eventData);

    void UpdateChildrenLayout();
    void EnsureRow(unsigned row);
    void EnsureCell(unsigned row, unsigned column);

    Vector<GridRow> rows_;

    unsigned childrenLayoutNestingLevel_ = 0;
    Vector<GridRowGroup> groupData_;
    Vector<int> minRowHeights_;
    int minWidth_ = 0;
    int minHeight_ = 0;
};

}
