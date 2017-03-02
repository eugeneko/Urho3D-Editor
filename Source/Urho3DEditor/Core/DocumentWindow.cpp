#include "DocumentWindow.h"
#include "Core.h"
#include "Document.h"
#include <QCloseEvent>

namespace Urho3DEditor
{

DocumentWindow::DocumentWindow(Core& core, Document* document, QWidget* parent /*= nullptr*/)
    : QMdiSubWindow(parent)
    , core_(core)
    , document_(document)
{
    assert(document);
    updateTitle();
    connect(document, &Document::titleChanged, this, &DocumentWindow::updateTitle);
    setWidget(document);
}

void DocumentWindow::updateTitle()
{
    setWindowTitle(document_->GetTitle());
}

void DocumentWindow::closeEvent(QCloseEvent* event)
{
    if (!core_.CloseDocument(*this))
        event->ignore();
}

}
