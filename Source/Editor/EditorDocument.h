#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>

namespace Urho3D
{

class Urho3DWidget;

class EditorDocument : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    EditorDocument(const QString& name);
    /// Destruct.
    virtual ~EditorDocument() {}
    /// Activate this document.
    void Activate();
    /// Deactivate this document.
    void Deactivate();
    /// Return whether the document is active.
    bool IsActive() const { return isActive_; }
    /// Get document name.
    const QString& GetName() { return name_; }

protected:
    /// Document activation handling.
    virtual void DoActivate() = 0;
    /// Document deactivation handling.
    virtual void DoDeactivate() = 0;

private:
    /// Document name.
    QString name_;
    /// Is document active?
    bool isActive_;
};

class Urho3DDocument : public EditorDocument, public Object
{
    Q_OBJECT
    URHO3D_OBJECT(Urho3DDocument, Object);

public:
    /// Construct.
    Urho3DDocument(Urho3DWidget* urho3dWidget, const QString& name);
    /// Destruct.
    virtual ~Urho3DDocument();
    /// Get Urho3D widget.
    Urho3DWidget* GetUrho3DWidget() const { return urho3dWidget_; }
    /// Get Urho3D context.
    Context* GetContext() const;

protected:
    /// Document activation handling.
    virtual void DoActivate() override;
    /// Document deactivation handling.
    virtual void DoDeactivate() override;

private:
    /// Urho3D widget.
    Urho3DWidget* urho3dWidget_;

};

class SceneDocument : public Urho3DDocument
{
    Q_OBJECT

public:
    /// Construct.
    SceneDocument(Urho3DWidget* urho3dWidget, const QString& name);
    /// Set scene.
    void SetScene(SharedPtr<Scene> scene);

protected:
    /// Document activation handling.
    virtual void DoActivate() override;

private:
    /// Setup viewport.
    void SetupViewport();

    /// Handle update.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle mouse button up/down.
    void HandleMouseButton(StringHash eventType, VariantMap& eventData);

private:
    /// Camera node.
    Node cameraNode_;
    /// Camera component.
    Camera* camera_;

    /// Scene.
    SharedPtr<Scene> scene_;
    /// Viewport.
    SharedPtr<Viewport> viewport_;
};

}

