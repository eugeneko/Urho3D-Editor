#include "Urho3DWidget.h"
#include "../Urho3DProject.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <QKeyEvent>

namespace Urho3DEditor
{

Urho3DWidget::Urho3DWidget(Urho3D::Context& context, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , Object(&context)
    , engine_(new Urho3D::Engine(context_))
{
    // Initialize engine
    Urho3D::VariantMap engineParameters = Urho3D::Engine::ParseParameters(Urho3D::GetArguments());
    engineParameters[Urho3D::EP_FULL_SCREEN] = false;
    engineParameters[Urho3D::EP_EXTERNAL_WINDOW] = (void*)winId();
    engine_->Initialize(engineParameters);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.start(16);
}

bool Urho3DWidget::SetCurrentProject(Urho3DProject* project)
{
    Urho3D::VariantMap engineParameters = Urho3D::Engine::ParseParameters(Urho3D::GetArguments());;
    if (project)
    {
        engineParameters[Urho3D::EP_RESOURCE_PREFIX_PATHS] = project->GetAbsoluteResourcePrefixPaths(project->GetBasePath()).toStdString().c_str();
        engineParameters[Urho3D::EP_RESOURCE_PATHS] = project->GetResourcePaths().toStdString().c_str();
    }
    return engine_->InitializeResourceCache(engineParameters);
}

void Urho3DWidget::OnTimer()
{
    RunFrame();
}

QPaintEngine* Urho3DWidget::paintEngine() const
{
    return nullptr;
}

void Urho3DWidget::paintEvent(QPaintEvent* /*event*/)
{
}

void Urho3DWidget::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event);
}

void Urho3DWidget::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased(event);
}

void Urho3DWidget::wheelEvent(QWheelEvent * event)
{
    emit wheelMoved(event);
}

void Urho3DWidget::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
}

void Urho3DWidget::RunFrame()
{
    if (!engine_->IsExiting())
        engine_->RunFrame();
}

//////////////////////////////////////////////////////////////////////////
Urho3DHost::Urho3DHost(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , context_(new Urho3D::Context())
    , urhoWidget_(new Urho3DWidget(*context_, this))
{
    setVisible(false);
}

void Urho3DHost::SetOwner(Urho3DClientWidget* client)
{
    client_ = client;
    if (!client_)
        urhoWidget_->setParent(this);
}

//////////////////////////////////////////////////////////////////////////
Urho3DClientWidget::Urho3DClientWidget(Urho3DHost& host, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , host_(host)
    , layout_(new QVBoxLayout(this))
    , placeholder_(new QLabel("Select to activate", this))
{
    placeholder_->setAlignment(Qt::AlignCenter);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(placeholder_);
    setLayout(layout_);
}

Urho3DClientWidget::~Urho3DClientWidget()
{
    Release();
}

void Urho3DClientWidget::Acquire()
{
    Urho3DClientWidget* oldOwner = host_.GetOwner();
    if (oldOwner == this)
        return;

    if (oldOwner)
        oldOwner->Release();
    host_.SetOwner(this);
    layout_->removeWidget(placeholder_);
    layout_->addWidget(host_.GetWidget());
}

void Urho3DClientWidget::Release()
{
    if (host_.GetOwner() != this)
        return;
    host_.SetOwner(nullptr);
    layout_->removeWidget(host_.GetWidget());
    layout_->addWidget(placeholder_);
}

}
