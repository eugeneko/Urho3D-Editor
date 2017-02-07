#pragma once

#include "../Module.h"
#include <QAction>
#include <QScopedPointer>

class QDockWidget;
class QGridLayout;

namespace Urho3DEditor
{

class Document;
class SceneDocument;

/// Attribute Inspector.
class AttributeInspector : public Module
{
    Q_OBJECT

public:
    /// Construct.
    AttributeInspector();

private:
    /// Initialize module.
    virtual bool Initialize() override;

private slots:
    /// Toggle show/hide.
    void ToggleShow(bool checked);
    /// Update menu.
    void UpdateMenu();
    /// Handle current document changed.
    void HandleCurrentDocumentChanged(Document* document);
    /// Handle selection changed.
    void HandleSelectionChanged();

private:
    /// Create body of inspector.
    void CreateBody();
    /// Create node panel.
    QGridLayout* CreateNodePanel();

private:
    /// Show action.
    QScopedPointer<QAction> showAction_;
    /// Main dock widget.
    QScopedPointer<QDockWidget> widget_;
    /// Current document.
    SceneDocument* document_;

};

}
