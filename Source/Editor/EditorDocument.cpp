#include "EditorDocument.h"
#include "Urho3DWidget.h"
// #include <Urho3D/Core/ProcessUtils.h>
// #include <Urho3D/Engine/EngineDefs.h>
// #include <QFile>
// #include <QHBoxLayout>
// #include <QTabBar>
// #include <QTimer>
#include <QVBoxLayout>
// 
// #include <Urho3D/Urho3DAll.h>

namespace Urho3D
{

EditorDocument::EditorDocument(const QString& name)
    : QWidget()
    , name_(name)
    , isActive_(false)
{

}

void EditorDocument::Activate()
{
    if (!isActive_)
    {
        isActive_ = true;
        DoActivate();
    }
}

void EditorDocument::Deactivate()
{
    if (isActive_)
    {
        isActive_ = false;
        DoDeactivate();
    }
}

//////////////////////////////////////////////////////////////////////////

Urho3DDocument::Urho3DDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : EditorDocument(name)
    , urho3dWidget_(urho3dWidget)
{
}

Urho3DDocument::~Urho3DDocument()
{
}

void Urho3DDocument::DoActivate()
{
    if (!layout())
        setLayout(new QVBoxLayout(this));
    layout()->addWidget(urho3dWidget_);
}

void Urho3DDocument::DoDeactivate()
{
    if (urho3dWidget_->parent() == this)
        urho3dWidget_->setParent(nullptr);
}

SceneDocument::SceneDocument(Urho3DWidget* urho3dWidget, const QString& name)
    : Urho3DDocument(urho3dWidget, name)
{

}

}
