#include "ProjectDocument.h"
#include "../Core/Core.h"
#include "../Core/QtUrhoHelpers.h"
// #include <QtXml/QDomDocument>
#include <QAction>
#include <QDirIterator>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>

namespace Urho3DEditor
{

ProjectDocument::ProjectDocument(Core& core)
    : Document(core)
    , project_(core.GetProject())
    , layout_(new QGridLayout(this))
    , projectFileName_(new QLineEdit(this))
    , resourcePrefixPaths_(new QLineEdit(this))
    , resourcePaths_(new QLineEdit(this))
    , autoloadPaths_(new QLineEdit(this))
    , packagePaths_(new QLineEdit(this))
    , defaultRenderPath_(new QLineEdit(this))
{
    projectFileName_->setReadOnly(true);

    setLayout(layout_);
    connect(resourcePrefixPaths_,   &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(resourcePaths_,         &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(autoloadPaths_,         &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(packagePaths_,          &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(defaultRenderPath_,     &QLineEdit::textEdited, this, [this]() { MarkDirty(); });

    int row = 0;
    layout_->addWidget(new QLabel(tr("Full project file name:")),   row, 0);
    layout_->addWidget(projectFileName_,                            row++, 1);
    layout_->addWidget(new QLabel(tr("Resource prefix paths:")),    row, 0);
    layout_->addWidget(resourcePrefixPaths_,                        row++, 1);
    layout_->addWidget(new QLabel(tr("Resource paths:")),           row, 0);
    layout_->addWidget(resourcePaths_,                              row++, 1);
    layout_->addWidget(new QLabel(tr("Autoload paths:")),           row, 0);
    layout_->addWidget(autoloadPaths_,                              row++, 1);
    layout_->addWidget(new QLabel(tr("Package paths:")),            row, 0);
    layout_->addWidget(packagePaths_,                               row++, 1);
    layout_->addWidget(new QLabel(tr("Default render path:")),      row, 0);
    layout_->addWidget(defaultRenderPath_,                          row++, 1);
    layout_->setRowStretch(row, 1);

    UpdateContent();
}

void ProjectDocument::UpdateContent()
{
    SetTitle(project_->GetTitle());
    projectFileName_->setText(project_->GetFileName());

    resourcePrefixPaths_->setText(project_->GetResourcePrefixPaths());
    resourcePaths_->setText(project_->GetResourcePaths());
    autoloadPaths_->setText(project_->GetAutoloadPaths());
    packagePaths_->setText(project_->GetPackagePaths());
    defaultRenderPath_->setText(project_->GetDefaultRenderPath());
}

bool ProjectDocument::DoSave(const QString& fileName)
{
    project_->SetResourcePrefixPaths(resourcePrefixPaths_->text());
    project_->SetResourcePaths(resourcePaths_->text());
    project_->SetAutoloadPaths(autoloadPaths_->text());
    project_->SetPackagePaths(packagePaths_->text());
    project_->SetDefaultRenderPath(defaultRenderPath_->text());

    project_->SetFileName(fileName);
    return project_->Save();
}

}
