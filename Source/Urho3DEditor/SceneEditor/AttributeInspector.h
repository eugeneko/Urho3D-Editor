#pragma once

#include "../Module.h"
#include "../Core/QtUrhoHelpers.h"
#include <QAction>
#include <QScopedPointer>

class QDockWidget;
class QGridLayout;

namespace Urho3DEditor
{

class Document;
class SceneDocument;
class SerializableWidget;

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

    /// Handle attribute changed (by any reason).
    void HandleAttributeChanged();
    /// Handle attribute edited (from editor widgets).
    void HandleAttributeEdited(const SerializableVector& serializables, unsigned attributeIndex,
        const QVector<Urho3D::Variant>& newValues);
    /// Handle attribute edit committed.
    void HandleAttributeEditCommitted();

private:
    /// Create body of inspector.
    void CreateBody();
    /// Create node panel.
    SerializableWidget* CreateNodePanel();
    /// Create node title string.
    QString CreateNodePanelTitle();

private:
    /// Show action.
    QScopedPointer<QAction> showAction_;
    /// Main dock widget.
    QScopedPointer<QDockWidget> widget_;
    /// Current document.
    SceneDocument* document_;

    /// Whether to suppress updates.
    bool suppressUpdates_;
    /// Serializable editors.
    QVector<SerializableWidget*> serializableEditors_;
    /// Generation of attribute changes. Changes within one generation are squashed.
    unsigned generation_;
};

}
