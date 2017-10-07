#include "GenericUI.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

GenericWidget::GenericWidget(AbstractUI& ui, GenericWidget* parent)
    : Object(ui.GetContext())
    , ui_(ui)
    , parent_(parent)
{

}

//////////////////////////////////////////////////////////////////////////
GenericWidget* GenericDialog::CreateBodyWidget(StringHash type)
{
    GenericWidget* widget = ui_.CreateWidget(type, this);
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

}
