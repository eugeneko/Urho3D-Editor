#include "Urho3DWidget.h"
#include "Urho3DProject.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
// #include <QFile>
// #include <QHBoxLayout>
// #include <QTabBar>
// #include <QTimer>

namespace Urho3D
{

Urho3DWidget::Urho3DWidget(Context* context)
    : QWidget()
    , Object(context)
    , engine_(MakeShared<Engine>(context_))
{
    // Initialize engine
    VariantMap engineParameters = Engine::ParseParameters(GetArguments());
    engineParameters[EP_FULL_SCREEN] = false;
    engineParameters[EP_EXTERNAL_WINDOW] = (void*)winId();
    engine_->Initialize(engineParameters);

    setAttribute(Qt::WA_PaintOnScreen);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.start(16);
}

bool Urho3DWidget::SetCurrentProject(Urho3DProject* project)
{
    VariantMap engineParameters = Engine::ParseParameters(GetArguments());;
    if (project)
    {
        engineParameters[EP_RESOURCE_PREFIX_PATHS] = project->GetAbsoluteResourcePrefixPaths(project->GetBasePath()).toStdString().c_str();
        engineParameters[EP_RESOURCE_PATHS] = project->GetResourcePaths().toStdString().c_str();
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

void Urho3DWidget::RunFrame()
{
    if (!engine_->IsExiting())
        engine_->RunFrame();
}

}
