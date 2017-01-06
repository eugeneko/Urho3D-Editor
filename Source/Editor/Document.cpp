#include "Document.h"
#include "Configuration.h"
#include "MainWindow.h"
#include <QFileDialog>

namespace Urho3DEditor
{

Document::Document(MainWindow& mainWindow)
    : mainWindow_(mainWindow)
{
    connect(&mainWindow, SIGNAL(pageChanged(Document*)), this, SLOT(HandleCurrentPageChanged(Document*)));
}

Document::~Document()
{

}

void Document::SetTitle(const QString& title)
{
    if (title != title_)
    {
        title_ = title;
        emit titleChanged(this);
    }
}

bool Document::LaunchFileDialog(bool open)
{
    QFileDialog dialog;
    dialog.setAcceptMode(open ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);
    dialog.setFileMode(open ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(mainWindow_.GetConfig().GetLastDirectory());
    dialog.setNameFilter(GetNameFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    mainWindow_.GetConfig().SetLastDirectoryByFileName(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool Document::Open()
{
    if (LaunchFileDialog(true))
        return DoLoad(fileName_);
    return false;
}

bool Document::IsActive() const
{
    return mainWindow_.GetCurrentPage() == this;
}

Configuration& Document::GetConfig()
{
    return mainWindow_.GetConfig();
}

void Document::HandleCurrentPageChanged(Document* document)
{

}

bool Document::DoLoad(const QString& /*fileName*/)
{
    return true;
}

}
