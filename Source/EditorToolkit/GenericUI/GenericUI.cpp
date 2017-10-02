#include "GenericUI.h"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/Scene.h>

namespace Urho3D
{

GenericWidget* GenericWidget::CreateChild(StringHash type)
{
    SharedPtr<GenericWidget> child(host_->CreateWidget(type));
    children_.Push(child);
    OnChildAdded(child);
    return child;
}

void GenericWidget::OnChildAdded(GenericWidget* widget)
{
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* GenericUIHost::CreateWidget(StringHash type)
{
    GenericWidget* widget = CreateWidgetImpl(type);
    if (widget)
        widget->SetHost(this);
    return widget;
}

}
