#include "AbstractDocument.h"
#include "EditorSettings.h"
#include "Widgets/Urho3DWidget.h"
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/IOEvents.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

namespace Urho3DEditor
{

AbstractPage::AbstractPage(Urho3D::Context* context)
    : Object(context)
    , unsavedChanges_(false)
{
}

bool AbstractPage::LaunchSaveDialog()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.selectFile(GetDefaultFileName());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    dialog.setNameFilter(GetFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    settings->SetLastDirectory(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool AbstractPage::Save(bool askForDestination)
{
    if (fileName_.isEmpty() || askForDestination)
    {
        if (!LaunchSaveDialog())
            return false;
    }
    if (DoSave())
    {
        ResetUnsaved();
        return true;
    }
    return false;
}

bool AbstractPage::LaunchOpenDialog()
{
    EditorSettings* settings = GetSubsystem<EditorSettings>();

    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(settings->GetLastDirectory());
    dialog.setNameFilter(GetFilters());
    if (!dialog.exec())
        return false;

    const QStringList files = dialog.selectedFiles();
    if (files.isEmpty())
        return false;

    fileName_ = files[0];
    settings->SetLastDirectory(fileName_);
    SetTitle(QFileInfo(fileName_).fileName());
    return true;
}

bool AbstractPage::Open(bool askForSource)
{
    if (fileName_.isEmpty() || askForSource)
    {
        if (!LaunchOpenDialog())
            return false;
    }
    ResetUnsaved();
    return DoLoad();
}

void AbstractPage::MarkUnsaved()
{
    const bool wasUnsaved = unsavedChanges_;
    unsavedChanges_ = true;
    if (!wasUnsaved)
        emit titleChanged(this, GetTitleDecorated());
}

void AbstractPage::ResetUnsaved()
{
    const bool wasUnsaved = unsavedChanges_;
    unsavedChanges_ = false;
    if (wasUnsaved)
        emit titleChanged(this, GetTitleDecorated());
}

void AbstractPage::SetTitle(const QString& value)
{
    title_ = value;
    emit titleChanged(this, GetTitleDecorated());
}

//////////////////////////////////////////////////////////////////////////
StartPage::StartPage(Urho3D::Context* context)
    : AbstractPage(context)
    , layout_(new QGridLayout(this))
    , newProjectButton_(new QPushButton("New Project"))
    , recentProjects_(new QListWidget())
{
    setLayout(layout_);
    layout_->addWidget(newProjectButton_, 0, 0);
    layout_->addWidget(new QLabel("Recent projects:"), 1, 0);
    layout_->addWidget(recentProjects_, 2, 0);
    SetTitle("Start Page");
}

}
