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
    /// Set central widget.
    void SetCentralWidget(QWidget* widget);

private slots:
    /// Handle collapsed state changed.
    void SetCollapsed(bool checked);

private:
    /// Update sizes.
    void UpdateSizes();

private:
    /// Main layout.
    QGridLayout* mainLayout_;
    /// Button.
    QToolButton* toggleButton_;
    /// Line.
    QFrame* headerLine_;
    /// Body.
    QScrollArea* body_;
    /// Is expanded?
    bool expanded_;
    /// Height of the header.
    int headerHeight_;
    /// Height of the body.
    int bodyHeight_;

};

}
