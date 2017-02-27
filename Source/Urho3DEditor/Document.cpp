#include "Document.h"
#include "Configuration.h"
#include "MainWindow.h"
#include <QFileDialog>

namespace Urho3DEditor
{

Document::Document(Core& core)
    : core_(core)
{
    connect(&core, SIGNAL(currentDocumentChanged(Document*)), this, SLOT(HandleCurrentDocumentChanged(Document*)));
}

Document::~Document()
{

}

void Document::MarkDirty()
{
    if (!dirty_)
    {
        dirty_ = true;
        emit titleChanged();
    }
}

void Document::ResetDirty()
{
    if (dirty_)
    {
        dirty_ = false;
        emit titleChanged();
    }
}

void Document::SetTitle(const QString& title)
{
    if (title != title_)
    {
        title_ = title;
        emit titleChanged();
    }
}

bool Document::Open(const QString& fileName)
{
    fileName_ = fileName;
    if (!DoLoad(fileName_))
        return false;
    SetTitle(QFileInfo(fileName_).fileName());
    ResetDirty();
    return true;
}

bool Document::Save(const QString& fileName)
{
    fileName_ = fileName;
    if (!DoSave(fileName_))
        return false;
    SetTitle(QFileInfo(fileName_).fileName());
    ResetDirty();
    return true;
}

bool Document::IsActive() const
{
    return core_.GetCurrentDocument() == this;
}

Configuration& Document::GetConfig()
{
    return core_.GetConfig();
}

void Document::HandleCurrentDocumentChanged(Document* document)
{

}

bool Document::DoLoad(const QString& /*fileName*/)
{
    return true;
}

bool Document::DoSave(const QString& /*fileName*/)
{
    return true;
}

}
