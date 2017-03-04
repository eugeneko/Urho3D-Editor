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
    , fieldResourcePrefixPaths_(new QLineEdit(this))
    , fieldResourcePaths_(new QLineEdit(this))
{
    projectFileName_->setReadOnly(true);

    setLayout(layout_);
    connect(fieldResourcePrefixPaths_, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(fieldResourcePaths_,       &QLineEdit::textEdited, this, [this]() { MarkDirty(); });

    int row = 0;
    layout_->addWidget(new QLabel(tr("Full project file name:")),   row, 0);
    layout_->addWidget(projectFileName_,                            row++, 1);
    layout_->addWidget(new QLabel(tr("Resource prefix paths:")),    row, 0);
    layout_->addWidget(fieldResourcePrefixPaths_,                   row++, 1);
    layout_->addWidget(new QLabel(tr("Resource paths:")),           row, 0);
    layout_->addWidget(fieldResourcePaths_,                         row++, 1);
    layout_->setRowStretch(3, 1);

    UpdateContent();
}

void ProjectDocument::UpdateContent()
{
    SetTitle(project_->GetTitle());
    projectFileName_->setText(project_->GetFileName());
    fieldResourcePrefixPaths_->setText(project_->GetResourcePrefixPaths());
    fieldResourcePaths_->setText(project_->GetResourcePaths());
}

bool ProjectDocument::DoSave(const QString& fileName)
{
    project_->SetResourcePrefixPaths(fieldResourcePrefixPaths_->text());
    project_->SetResourcePaths(fieldResourcePaths_->text());
    project_->SetFileName(fileName);
    return project_->Save();
}

}
