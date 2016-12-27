#include "Urho3DWidget.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
// #include <QFile>
// #include <QHBoxLayout>
// #include <QTabBar>
// #include <QTimer>

#include <Urho3D/Urho3DAll.h>

namespace Urho3D
{

Urho3DWidget::Urho3DWidget(Context* context)
    : QWidget()
    , Object(context)
    , engine_(MakeShared<Engine>(context))
{
    setAttribute(Qt::WA_PaintOnScreen);

    VariantMap engineParameters = Engine::ParseParameters(GetArguments());
    engineParameters[EP_FULL_SCREEN] = false;
//     engineParameters[EP_BORDERLESS] = true;
//     engineParameters[EP_WINDOW_RESIZABLE] = true;
    engineParameters[EP_EXTERNAL_WINDOW] = (void*)winId();

    if (engine_->Initialize(engineParameters))
    {
        connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
        timer_.start(16);
    }

    //////////////////////////////////////////////////////////////////////////
    /*
    Scene* scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // Create camera.
    Node* cameraNode_ = new Node(context_);
    Camera* camera = cameraNode_->CreateComponent<Camera>();

    camera->SetOrthographic(true);
    Graphics* graphic = context_->GetSubsystem<Graphics>();
    camera->SetOrthoSize(graphic->GetHeight() * PIXEL_SIZE);

    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));

    Renderer* renderer = context_->GetSubsystem<Renderer>();
    renderer->SetViewport(0, viewport);
    */
    //////////////////////////////////////////////////////////////////////////

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

void Urho3DWidget::RunFrame()
{
    if (engine_ && !engine_->IsExiting())
        engine_->RunFrame();
}

}
