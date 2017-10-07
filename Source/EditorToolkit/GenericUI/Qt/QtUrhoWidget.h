#pragma once

#include "../UrhoUI.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/PackageFile.h>
#include <QSet>
#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

namespace Urho3D
{

/// Qt input.
class QtInput : public UrhoInput
{
public:
    /// Construct.
    QtInput(Context* context);

    /// Process key press.
    void OnKeyPress(Qt::Key key);
    /// Process key release.
    void OnKeyRelease(Qt::Key key);
    /// Process mouse wheel.
    void OnWheel(int delta);
    /// Process focus out.
    void OnFocusOut();
    /// End frame.
    void EndFrame();

    /// \see AbstractInput::IsUIHovered
    bool IsKeyDown(int key) const override;
    /// \see AbstractInput::IsUIHovered
    bool IsKeyPressed(int key) const override;
    /// \see AbstractInput::IsUIHovered
    int GetMouseWheelMove() const override;

private:
    /// Keys pressed.
    HashSet<int> keysPressed_;
    /// Keys down.
    HashSet<int> keysDown_;
    /// Mouse wheel delta.
    int mouseWheelDelta_ = 0;

};

/// Urho3D widget that owns context and all systems.
class QtUrhoWidget : public QWidget, public Object
{
    Q_OBJECT
    URHO3D_OBJECT(QtUrhoWidget, Object);

public:
    /// Construct.
    QtUrhoWidget(Context& context, QWidget* parent = nullptr);
    /// Initialize Urho3D systems. If systems are already initialized, partial initialization is performed.
    bool Initialize(VariantMap parameters);
    /// Clear resource cache.
    void ClearResourceCache();
    /// Set resource cache folders.
    bool SetResourceCache(const VariantMap& parameters);
    /// Set default render path.
    bool SetDefaultRenderPath(const QString& fileName);
    /// Returns whether the Urho3D systems initialized.
    bool IsInitialized() const { return engine_->IsInitialized(); }
    /// Get input.
    AbstractInput* GetInput() { return input_; }

private slots:
    /// Handle main timer.
    void OnTimer();

protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void RunFrame();

private:
    /// Engine.
    SharedPtr<Engine> engine_;
    /// Main timer.
    QTimer timer_;
    /// Input.
    SharedPtr<QtInput> input_;

};

}

