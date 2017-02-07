#pragma once

#include <QWidget>

class QFrame;
class QGridLayout;
class QScrollArea;
class QToolButton;

namespace Urho3DEditor
{

// Collapsible Section Widget
class CollapsiblePanelWidget : public QWidget
{
    Q_OBJECT

public:
    /// Construct.
    CollapsiblePanelWidget(const QString& title = "", bool expanded = false, QWidget* parent = 0);
    /// Set content layout.
    void SetContentLayout(QLayout* contentLayout);

private slots:
    /// Handle collapsed state changed.
    void SetCollapsed(bool checked);

private:
    /// Main layout.
    QScopedPointer<QGridLayout> mainLayout_;
    /// Button.
    QScopedPointer<QToolButton> toggleButton_;
    /// Line.
    QScopedPointer<QFrame> headerLine_;
    /// Body.
    QScopedPointer<QScrollArea> body_;
};

}
