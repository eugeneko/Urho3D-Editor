#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
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

class Urho3DDocument : public EditorDocument
{
    Q_OBJECT

public:
    /// Construct.
    Urho3DDocument(Urho3DWidget* urho3dWidget, const QString& name);
    /// Destruct.
    virtual ~Urho3DDocument();

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

private:
    /// Name of the document.
    QString name_;
};

}

