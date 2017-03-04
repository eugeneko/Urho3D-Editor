#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/PackageFile.h>
#include <QSet>
#include <QTimer>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

namespace Urho3DEditor
{

/// Urho3D widget that owns context and all systems.
class Urho3DWidget : public QWidget, public Urho3D::Object
{
    Q_OBJECT
    URHO3D_OBJECT(Urho3DWidget, Urho3D::Object);

public:
    /// Construct.
    Urho3DWidget(Urho3D::Context& context, QWidget* parent = nullptr);
    /// Initialize Urho3D systems. If systems are already initialized, partial initialization is performed.
    bool Initialize(Urho3D::VariantMap parameters);
    /// Clear resource cache.
    void ClearResourceCache();
    /// Set resource cache folders.
    bool SetResourceCache(const Urho3D::VariantMap& parameters);
    /// Set default render path.
    bool SetDefaultRenderPath(const QString& fileName);
    /// Returns whether the Urho3D systems initialized.
    bool IsInitialized() const { return engine_->IsInitialized(); }

signals:
    /// Signals that key is pressed.
    void keyPressed(QKeyEvent* event);
    /// Signals that key is released.
    void keyReleased(QKeyEvent* event);
    /// Signals that wheel is moved.
    void wheelMoved(QWheelEvent* event);
    /// Signals that widget lost focus.
    void focusOut();

private slots:
    /// Handle main timer.
    void OnTimer();

protected:
    virtual QPaintEngine * paintEngine() const override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void wheelEvent(QWheelEvent * event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;

private:
    void RunFrame();

private:
    /// Engine.
    Urho3D::SharedPtr<Urho3D::Engine> engine_;
    /// Main timer.
    QTimer timer_;

};

class Urho3DClientWidget;

/// Urho3D host that holds Urho3D widget. Shan't be used as display widget.
class Urho3DHost : private QWidget
{
    Q_OBJECT

public:
    /// Construct.
    Urho3DHost(QWidget* parent = nullptr);
    /// Initialize Urho3D systems. If systems are already initialized, partial initialization is performed.
    bool Initialize(const Urho3D::VariantMap& parameters) { return urhoWidget_->Initialize(parameters); }
    /// Get widget.
    Urho3DWidget& GetWidget() const { return *urhoWidget_; }

    /// Get current client.
    Urho3DClientWidget* GetOwner() const { return client_; }
    /// Set current client.
    void SetOwner(Urho3DClientWidget* client);

private:
    using QWidget::setVisible;

    /// Urho3D context.
    Urho3D::SharedPtr<Urho3D::Context> context_;
    /// Urho3D widget.
    QScopedPointer<Urho3DWidget> urhoWidget_;
    /// Owner.
    Urho3DClientWidget* client_ = nullptr;
};

/// Urho3D client widget. Multiple clients may share single host. However, only one client is able to display host content.
class Urho3DClientWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    Urho3DClientWidget(Urho3DHost& host, QWidget* parent = nullptr);
    /// Destroy.
    virtual ~Urho3DClientWidget();

    /// Acquire ownership over host.
    void Acquire();
    /// Release ownership.
    void Release();

private:
    /// Host.
    Urho3DHost& host_;
    /// Layout.
    QVBoxLayout* layout_;
    /// Placeholder.
    QLabel* placeholder_;
};

}

