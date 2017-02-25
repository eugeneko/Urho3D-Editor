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

bool Document::LaunchFileDialog(bool open)
{
    QFileDialog dialog;
    if (!open)
        dialog.selectFile(GetDefaultName());
    dialog.setAcceptMode(open ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);
    dialog.setFileMode(open ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(core_.GetConfig().GetLastDirectory());
    dialog.setNameFilter(GetNameFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    core_.GetConfig().SetLastDirectoryByFileName(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool Document::Open()
{
    if (!LaunchFileDialog(true))
        return false;
    return DoLoad(fileName_);
}

bool Document::SaveAs()
{
    if (!LaunchFileDialog(false))
        return false;
    if (DoSave(fileName_))
    {
        ResetDirty();
        return true;
    }
    return false;
}

bool Document::Save()
{
    if (!fileName_.isEmpty() && DoSave(fileName_))
    {
        ResetDirty();
        return true;
    }
    return SaveAs();
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
