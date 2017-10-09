#include "GenericUI.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

GenericWidget::GenericWidget(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : Object(mainWindow.GetContext())
    , mainWindow_(mainWindow)
    , parent_(parent)
{

}

//////////////////////////////////////////////////////////////////////////
GenericWidget* GenericDialog::CreateBodyWidget(StringHash type)
{
    SharedPtr<GenericWidget> widget = mainWindow_.CreateWidget(type, this);
    SetBodyWidget(widget);
    return widget;
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

SharedPtr<GenericWidget> AbstractMainWindow::CreateWidget(StringHash type, GenericWidget* parent)
{
    using WidgetFactory = SharedPtr<GenericWidget>(AbstractMainWindow::*)(GenericWidget* parent);
    static const HashMap<StringHash, WidgetFactory> factory =
    {
        { AbstractLayout::GetTypeStatic(), &AbstractMainWindow::CreateLayout },
        { AbstractButton::GetTypeStatic(), &AbstractMainWindow::CreateButton},
        { AbstractText::GetTypeStatic(), &AbstractMainWindow::CreateText },
        { AbstractLineEdit::GetTypeStatic(), &AbstractMainWindow::CreateLineEdit },
        { GenericHierarchyList::GetTypeStatic(), &AbstractMainWindow::CreateHierarchyList },
    };

    WidgetFactory createWidget = nullptr;
    factory.TryGetValue(type, createWidget);
    return createWidget ? (this->*createWidget)(parent) : nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateLayout(GenericWidget* parent)
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateButton(GenericWidget* parent)
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateText(GenericWidget* parent)
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateLineEdit(GenericWidget* parent)
{
    return nullptr;
}

SharedPtr<GenericWidget> AbstractMainWindow::CreateHierarchyList(GenericWidget* parent)
{
    return nullptr;
}

}
