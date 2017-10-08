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

QtInput::QtInput(Context* context)
    : StandardUrhoInput(context)
{

}

void QtInput::OnKeyPress(Qt::Key key)
{
    const int urhoKey = Cast(key);
    keysPressed_.Insert(urhoKey);
    keysDown_.Insert(urhoKey);
}

void QtInput::OnKeyRelease(Qt::Key key)
{
    const int urhoKey = Cast(key);
    keysDown_.Erase(urhoKey);
}

void QtInput::OnWheel(int delta)
{
    mouseWheelDelta_ += delta / 120;
}

void QtInput::OnFocusOut()
{
    keysDown_.Clear();
}

void QtInput::EndFrame()
{
    mouseWheelDelta_ = 0;
    keysPressed_.Clear();
}

bool QtInput::IsKeyDown(int key) const
{
    return keysDown_.Contains(key);
}

bool QtInput::IsKeyPressed(int key) const
{
    return keysPressed_.Contains(key);
}

int QtInput::GetMouseWheelMove() const
{
    return mouseWheelDelta_;
}

//////////////////////////////////////////////////////////////////////////
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
    if (engine_->IsInitialized())
        return true;

    if (!engine_->Initialize(parameters))
        return false;

    input_ = MakeShared<QtInput>(context_);
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

void QtUrhoWidget::keyPressEvent(QKeyEvent* event)
{
    if (input_)
        input_->OnKeyPress(Qt::Key(event->key()));
}

void QtUrhoWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (input_)
        input_->OnKeyRelease(Qt::Key(event->key()));
}

void QtUrhoWidget::wheelEvent(QWheelEvent* event)
{
    if (input_)
        input_->OnWheel(event->delta());
}

void QtUrhoWidget::focusOutEvent(QFocusEvent* event)
{
    if (input_)
        input_->OnFocusOut();
}

void QtUrhoWidget::RunFrame()
{
    if (engine_->IsInitialized() && !engine_->IsExiting())
    {
        engine_->RunFrame();
        input_->EndFrame();
    }
}

}
