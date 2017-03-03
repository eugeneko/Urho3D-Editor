#include "Urho3DWidget.h"
#include "../Core/QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Renderer.h>
#include <QKeyEvent>

namespace Urho3DEditor
{

Urho3DWidget::Urho3DWidget(Urho3D::Context& context, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , Object(&context)
    , engine_(new Urho3D::Engine(context_))
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.start(16);
}

bool Urho3DWidget::Initialize(Urho3D::VariantMap parameters)
{
    // Override some parameters
    parameters[Urho3D::EP_FULL_SCREEN] = false;
    parameters[Urho3D::EP_EXTERNAL_WINDOW] = (void*)winId();

    // Initialize engine
    if (!engine_->IsInitialized())
        return engine_->Initialize(parameters);
    else
        return true;
}

bool Urho3DWidget::SetResourceCache(const Urho3D::VariantMap& parameters)
{
    if (!engine_->IsInitialized())
        return false;
    Urho3D::ResourceCache* cache = engine_->GetSubsystem<Urho3D::ResourceCache>();
    cache->ReleaseAllResources(true);
    return engine_->InitializeResourceCache(parameters);
}

bool Urho3DWidget::SetDefaultRenderPath(const QString& fileName)
{
    if (Urho3D::Renderer* renderer = engine_->GetSubsystem<Urho3D::Renderer>())
    {
        Urho3D::ResourceCache* resourceCache = engine_->GetSubsystem<Urho3D::ResourceCache>();
        if (Urho3D::XMLFile* renderPath = resourceCache->GetResource<Urho3D::XMLFile>(Cast(fileName)))
        {
            renderer->SetDefaultRenderPath(renderPath);
            return true;
        }
    }
    return false;
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
    if (engine_->IsInitialized() && !engine_->IsExiting())
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
    layout_->addWidget(&host_.GetWidget());
}

void Urho3DClientWidget::Release()
{
    if (host_.GetOwner() != this)
        return;
    host_.SetOwner(nullptr);
    layout_->removeWidget(&host_.GetWidget());
    layout_->addWidget(placeholder_);
}

}
