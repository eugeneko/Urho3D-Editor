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

namespace
{

QMap<Qt::Key, int> BuildKeyMap()
{
    QMap<Qt::Key, int> keymap;
    keymap[Qt::Key_unknown] = KEY_UNKNOWN;
    keymap[Qt::Key_Escape] = KEY_ESCAPE;
    keymap[Qt::Key_Tab] = KEY_TAB;
    keymap[Qt::Key_Backspace] = KEY_BACKSPACE;
    keymap[Qt::Key_Return] = KEY_RETURN;
    keymap[Qt::Key_Enter] = KEY_KP_ENTER;
    keymap[Qt::Key_Insert] = KEY_INSERT;
    keymap[Qt::Key_Delete] = KEY_DELETE;
    keymap[Qt::Key_Pause] = KEY_PAUSE;
    keymap[Qt::Key_Print] = KEY_PRINTSCREEN;
    keymap[Qt::Key_Home] = KEY_HOME;
    keymap[Qt::Key_End] = KEY_END;
    keymap[Qt::Key_Left] = KEY_LEFT;
    keymap[Qt::Key_Right] = KEY_RIGHT;
    keymap[Qt::Key_Up] = KEY_UP;
    keymap[Qt::Key_Down] = KEY_DOWN;
    keymap[Qt::Key_PageUp] = KEY_PAGEUP;
    keymap[Qt::Key_PageDown] = KEY_PAGEDOWN;
    keymap[Qt::Key_Shift] = KEY_LSHIFT;
    keymap[Qt::Key_Control] = KEY_LCTRL;
    keymap[Qt::Key_Alt] = KEY_LALT;
    keymap[Qt::Key_CapsLock] = KEY_CAPSLOCK;
    keymap[Qt::Key_NumLock] = KEY_NUMLOCKCLEAR;
    keymap[Qt::Key_ScrollLock] = KEY_SCROLLLOCK;
    keymap[Qt::Key_F1] = KEY_F1;
    keymap[Qt::Key_F2] = KEY_F2;
    keymap[Qt::Key_F3] = KEY_F3;
    keymap[Qt::Key_F4] = KEY_F4;
    keymap[Qt::Key_F5] = KEY_F5;
    keymap[Qt::Key_F6] = KEY_F6;
    keymap[Qt::Key_F7] = KEY_F7;
    keymap[Qt::Key_F8] = KEY_F8;
    keymap[Qt::Key_F9] = KEY_F9;
    keymap[Qt::Key_F10] = KEY_F10;
    keymap[Qt::Key_F11] = KEY_F11;
    keymap[Qt::Key_F12] = KEY_F12;
    keymap[Qt::Key_F13] = KEY_F13;
    keymap[Qt::Key_F14] = KEY_F14;
    keymap[Qt::Key_F15] = KEY_F15;

    for (int key = 'A'; key <= 'Z'; key++)
        keymap[Qt::Key(key)] = KEY_A + key - 'A';

    for (int key = '0'; key <= '9'; key++)
        keymap[Qt::Key(key)] = KEY_0 + key - '0';

    return keymap;
}

int Cast(Qt::Key key)
{
    static const QMap<Qt::Key, int> keymap = BuildKeyMap();
    return keymap.value(key, KEY_UNKNOWN);
}

}

//////////////////////////////////////////////////////////////////////////
QtInput::QtInput(Context* context)
    : UrhoInput(context)
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
