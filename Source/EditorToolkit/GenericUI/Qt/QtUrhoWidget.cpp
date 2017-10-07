#include "QtUrhoWidget.h"
#include "QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Renderer.h>
#include <QKeyEvent>

namespace Urho3D
{

QtUrhoWidget::QtUrhoWidget(Context& context, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , Object(&context)
    , engine_(new Engine(context_))
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.start(16);
}

bool QtUrhoWidget::Initialize(VariantMap parameters)
{
    // Override some parameters
    parameters[EP_FULL_SCREEN] = false;
    parameters[EP_EXTERNAL_WINDOW] = (void*)winId();

    // Initialize engine
    if (!engine_->IsInitialized())
        return engine_->Initialize(parameters);
    else
        return true;
}

void QtUrhoWidget::ClearResourceCache()
{
    if (ResourceCache* cache = engine_->GetSubsystem<ResourceCache>())
        cache->ReleaseAllResources(true);

}

bool QtUrhoWidget::SetResourceCache(const VariantMap& parameters)
{
    if (!engine_->IsInitialized())
        return false;
    return engine_->InitializeResourceCache(parameters);
}

bool QtUrhoWidget::SetDefaultRenderPath(const QString& fileName)
{
    if (Renderer* renderer = engine_->GetSubsystem<Renderer>())
    {
        ResourceCache* resourceCache = engine_->GetSubsystem<ResourceCache>();
        if (XMLFile* renderPath = resourceCache->GetResource<XMLFile>(Cast(fileName)))
        {
            renderer->SetDefaultRenderPath(renderPath);
            return true;
        }
    }
    return false;
}

void QtUrhoWidget::OnTimer()
{
    RunFrame();
}

QPaintEngine* QtUrhoWidget::paintEngine() const
{
    return nullptr;
}

void QtUrhoWidget::paintEvent(QPaintEvent* /*event*/)
{
}

void QtUrhoWidget::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event);
}

void QtUrhoWidget::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased(event);
}

void QtUrhoWidget::wheelEvent(QWheelEvent * event)
{
    emit wheelMoved(event);
}

void QtUrhoWidget::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
}

void QtUrhoWidget::RunFrame()
{
    if (engine_->IsInitialized() && !engine_->IsExiting())
        engine_->RunFrame();
}

//////////////////////////////////////////////////////////////////////////
QtUrhoHost::QtUrhoHost(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , context_(new Context())
    , urhoWidget_(new QtUrhoWidget(*context_, this))
{
    setVisible(false);
}

void QtUrhoHost::SetOwner(QtUrhoClientWidget* client)
{
    client_ = client;
    if (!client_)
        urhoWidget_->setParent(this);
}

//////////////////////////////////////////////////////////////////////////
QtUrhoClientWidget::QtUrhoClientWidget(QtUrhoHost& host, QWidget* parent /*= nullptr*/)
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

QtUrhoClientWidget::~QtUrhoClientWidget()
{
    Release();
}

void QtUrhoClientWidget::Acquire()
{
    QtUrhoClientWidget* oldOwner = host_.GetOwner();
    if (oldOwner == this)
        return;

    if (oldOwner)
        oldOwner->Release();
    host_.SetOwner(this);
    layout_->removeWidget(placeholder_);
    layout_->addWidget(&host_.GetWidget());
}

void QtUrhoClientWidget::Release()
{
    if (host_.GetOwner() != this)
        return;
    host_.SetOwner(nullptr);
    layout_->removeWidget(&host_.GetWidget());
    layout_->addWidget(placeholder_);
}

}
