#include "GenericUI.h"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

GenericWidget::GenericWidget(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
    , mainWindow_(mainWindow)
{

}

void GenericWidget::SetParent(GenericWidget* parent)
{
    parent_ = parent;
    OnParentSet();
}

//////////////////////////////////////////////////////////////////////////
bool GenericDialog::SetContent(GenericWidget* content)
{
    content_ = content;
    content_->SetParent(this);

    if (!DoSetContent(content))
    {
        content_ = nullptr;
        return false;
    }
    return true;
}

GenericWidget* GenericDialog::CreateContent(StringHash type)
{
    SharedPtr<GenericWidget> content = mainWindow_.CreateWidget(type);
    SetContent(content);
    return content_;
}

//////////////////////////////////////////////////////////////////////////
bool AbstractScrollArea::SetContent(GenericWidget* content)
{
    content_ = content;
    content_->SetParent(this);

    if (!DoSetContent(content))
    {
        content_ = nullptr;
        return false;
    }
    return true;
}

GenericWidget* AbstractScrollArea::CreateContent(StringHash type)
{
    SharedPtr<GenericWidget> content = mainWindow_.CreateWidget(type);
    SetContent(content);
    return content_;
}

//////////////////////////////////////////////////////////////////////////
bool AbstractLayout::SetCell(GenericWidget* cell, unsigned row, unsigned column)
{
    if (!cell)
        return false;
    if (!EnsureCell(row, column, RowType::MultiCellRow))
    {
        URHO3D_LOGERRORF("Cannot set column %u at row %u because the row has incompatible type", column, row);
        return false;
    }
    if (rows_[row].columns_[column])
    {
        URHO3D_LOGERRORF("Cannot set column %u at row %u because the cell is already set", column, row);
        return false;
    }
    rows_[row].columns_[column] = cell;
    cell->SetParent(this);
    DoSetCell(row, column, cell);
    return true;
}

GenericWidget* AbstractLayout::CreateCell(StringHash type, unsigned row, unsigned column)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type);
    if (!SetCell(child, row, column))
        return nullptr;
    return child;
}

bool AbstractLayout::SetRow(GenericWidget* cell, unsigned row)
{
    if (!cell)
        return false;
    if (!EnsureRow(row, RowType::SimpleRow))
    {
        URHO3D_LOGERRORF("Cannot set row %u because the row has incompatible type", row);
        return false;
    }
    if (rows_[row].columns_[0])
    {
        URHO3D_LOGERRORF("Cannot set row %u because the row is already set", row);
        return false;
    }
    rows_[row].columns_[0] = cell;
    cell->SetParent(this);
    DoSetRow(row, cell);
    return true;
}

GenericWidget* AbstractLayout::CreateRow(StringHash type, unsigned row)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type);
    if (!SetRow(child, row))
        return nullptr;
    return child;
}

void AbstractLayout::RemoveRow(unsigned row)
{
    if (row >= rows_.Size())
        return;

    Vector<RowData> rowsCopy = rows_;
    rows_[row].type_ = RowType::EmptyRow;
    rows_[row].columns_.Clear();

    for (GenericWidget* cell : rowsCopy[row].columns_)
    {
        if (cell)
            DoRemoveChild(cell);
    }
}

void AbstractLayout::RemoveAllChildren()
{
    Vector<RowData> rowsCopy = rows_;
    rows_.Clear();

    for (RowData& row : rowsCopy)
    {
        for (GenericWidget* cell : row.columns_)
        {
            if (cell)
                DoRemoveChild(cell);
        }
    }
}

bool AbstractLayout::EnsureRow(unsigned row, RowType type)
{
    if (rows_.Size() <= row)
        rows_.Resize(row + 1);
    if (rows_[row].type_ == RowType::EmptyRow)
        rows_[row].type_ = type;
    if (rows_[row].type_ == RowType::SimpleRow && rows_[row].columns_.Empty())
        rows_[row].columns_.Push(nullptr);
    return rows_[row].type_ == type;
}

bool AbstractLayout::EnsureCell(unsigned row, unsigned column, RowType type)
{
    const bool rowValid = EnsureRow(row, type);
    if (rows_[row].columns_.Size() <= column)
        rows_[row].columns_.Resize(column + 1);
    return rowValid;
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* AbstractCollapsiblePanel::CreateHeaderPrefix(StringHash type)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type);
    SetHeaderPrefix(child);
    return headerPrefix_;
}

GenericWidget* AbstractCollapsiblePanel::CreateHeaderSuffix(StringHash type)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type);
    SetHeaderSuffix(child);
    return headerSuffix_;
}

GenericWidget* AbstractCollapsiblePanel::CreateBody(StringHash type)
{
    SharedPtr<GenericWidget> child = mainWindow_.CreateWidget(type);
    SetBody(child);
    return body_;
}

bool AbstractCollapsiblePanel::SetHeaderPrefix(GenericWidget* header)
{
    headerPrefix_ = header;
    headerPrefix_->SetParent(this);
    if (!DoSetHeaderPrefix(header))
    {
        headerPrefix_ = nullptr;
        return false;
    }
    return true;
}

bool AbstractCollapsiblePanel::SetHeaderSuffix(GenericWidget* header)
{
    headerSuffix_ = header;
    headerSuffix_->SetParent(this);
    if (!DoSetHeaderSuffix(header))
    {
        headerSuffix_ = nullptr;
        return false;
    }
    return true;
}

bool AbstractCollapsiblePanel::SetBody(GenericWidget* body)
{
    body_ = body;
    body_->SetParent(this);
    if (!DoSetBody(body))
    {
        body_ = nullptr;
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
void GenericHierarchyListItem::InsertChild(GenericHierarchyListItem* item, unsigned index)
{
    children_.Insert(index, SharedPtr<GenericHierarchyListItem>(item));
    item->SetParent(this);
}

void GenericHierarchyListItem::RemoveChild(unsigned index)
{
    children_.Erase(index);
}

int GenericHierarchyListItem::GetIndex()
{
    if (parent_)
    {
        const unsigned idx = parent_->children_.IndexOf(SharedPtr<GenericHierarchyListItem>(this));
        return idx < parent_->children_.Size() ? static_cast<int>(idx) : -1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
SharedPtr<GenericWidget> AbstractMainWindow::CreateWidget(StringHash type)
{
    using WidgetFactory = SharedPtr<GenericWidget>(AbstractMainWindow::*)();
    static const HashMap<StringHash, WidgetFactory> factory =
    {
        { AbstractDummyWidget::GetTypeStatic(), &AbstractMainWindow::CreateDummyWidget },
        { AbstractScrollArea::GetTypeStatic(), &AbstractMainWindow::CreateScrollArea },
        { AbstractLayout::GetTypeStatic(), &AbstractMainWindow::CreateLayout },
        { AbstractCollapsiblePanel::GetTypeStatic(), &AbstractMainWindow::CreateCollapsiblePanel },
        { AbstractButton::GetTypeStatic(), &AbstractMainWindow::CreateButton},
        { AbstractText::GetTypeStatic(), &AbstractMainWindow::CreateText },
        { AbstractLineEdit::GetTypeStatic(), &AbstractMainWindow::CreateLineEdit },
        { AbstractCheckBox::GetTypeStatic(), &AbstractMainWindow::CreateCheckBox },
        { GenericHierarchyList::GetTypeStatic(), &AbstractMainWindow::CreateHierarchyList },
    };

    WidgetFactory createWidget = nullptr;
    factory.TryGetValue(type, createWidget);
    return createWidget ? (this->*createWidget)() : nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateDummyWidget()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateScrollArea()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateLayout()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateCollapsiblePanel()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateButton()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateText()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateLineEdit()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateCheckBox()
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateHierarchyList()
{
    return nullptr;
}

}
