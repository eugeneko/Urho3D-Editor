#pragma once

#include <QMdiSubWindow>

namespace Urho3DEditor
{

class Document;
class Core;

/// Document window. Owned document mustn't be null and destroyed in destructor.
class DocumentWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    /// Construct.
    DocumentWindow(Core& core, Document* document, QWidget* parent = nullptr);
    /// Get document.
    Document& GetDocument() const { return *document_; }

private slots:
    /// Update title.
    void updateTitle();

private:
    /// @see QWidget::closeEvent
    virtual void closeEvent(QCloseEvent *event) override;

private:
    /// Core.
    Core& core_;
    /// Document.
    QScopedPointer<Document> document_;

};

}

