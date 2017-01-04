#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/PackageFile.h>
#include <QSet>
#include <QTimer>
#include <QWidget>

namespace Urho3DEditor
{

class Urho3DProject;

class Urho3DWidget : public QWidget, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(Urho3DWidget, Urho3D::Object);

public:
    /// Construct.
    Urho3DWidget(Urho3D::Context& context);
    /// Initialize widget with optional configuration.
    bool SetCurrentProject(Urho3DProject* project);

    /// Check whether the key is pressed.
    bool IsKeyPressed(int key) const { return pressedKeys_.contains(key); }

private slots:
    /// Handle main timer.
    void OnTimer();

protected:
    virtual QPaintEngine * paintEngine() const override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;

private:
    void RunFrame();

private:
    /// Engine.
    Urho3D::SharedPtr<Urho3D::Engine> engine_;
    /// Main timer.
    QTimer timer_;
    /// Pressed keys.
    QSet<int> pressedKeys_;

};

}

