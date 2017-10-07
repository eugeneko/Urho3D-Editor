#pragma once

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
    SharedPtr<Engine> engine_;
    /// Main timer.
    QTimer timer_;

};

class QtUrhoClientWidget;

/// Urho3D host that holds Urho3D widget. Shan't be used as display widget.
class QtUrhoHost : private QWidget
{
    Q_OBJECT

public:
    /// Construct.
    QtUrhoHost(QWidget* parent = nullptr);
    /// Initialize Urho3D systems. If systems are already initialized, partial initialization is performed.
    bool Initialize(const VariantMap& parameters) { return urhoWidget_->Initialize(parameters); }
    /// Get widget.
    QtUrhoWidget& GetWidget() const { return *urhoWidget_; }

    /// Get current client.
    QtUrhoClientWidget* GetOwner() const { return client_; }
    /// Set current client.
    void SetOwner(QtUrhoClientWidget* client);

private:
    using QWidget::setVisible;

    /// Urho3D context.
    SharedPtr<Context> context_;
    /// Urho3D widget.
    QScopedPointer<QtUrhoWidget> urhoWidget_;
    /// Owner.
    QtUrhoClientWidget* client_ = nullptr;
};

/// Urho3D client widget. Multiple clients may share single host. However, only one client is able to display host content.
class QtUrhoClientWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    QtUrhoClientWidget(QtUrhoHost& host, QWidget* parent = nullptr);
    /// Destroy.
    virtual ~QtUrhoClientWidget();

    /// Acquire ownership over host.
    void Acquire();
    /// Release ownership.
    void Release();

private:
    /// Host.
    QtUrhoHost& host_;
    /// Layout.
    QVBoxLayout* layout_;
    /// Placeholder.
    QLabel* placeholder_;
};

}

