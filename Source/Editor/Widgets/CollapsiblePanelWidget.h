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
    CollapsiblePanelWidget(const QString& title = "", bool expanded = false, QWidget* parent = nullptr);
    /// Set content layout.
    void SetContentLayout(QLayout* contentLayout);

private slots:
    /// Handle collapsed state changed.
    void SetCollapsed(bool checked);

private:
    /// Main layout.
    QGridLayout* mainLayout_;
    /// Button.
    QToolButton* toggleButton_;
    /// Line.
    QFrame* headerLine_;
    /// Body.
    QScrollArea* body_;
};

}
