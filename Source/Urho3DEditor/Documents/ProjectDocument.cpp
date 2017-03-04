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
    , project_(new Project())
    , layout_(new QGridLayout())
    , buttonSetAsCurrent_(new QPushButton("Wear this project"))
    , fieldResourcePrefixPaths_(new QLineEdit("."))
    , fieldResourcePaths_(new QLineEdit("CoreData;Data"))
{
    SetTitle("New Project");
    MarkDirty();

    setLayout(layout_);
    connect(fieldResourcePrefixPaths_, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(fieldResourcePaths_,       &QLineEdit::textEdited, this, [this]() { MarkDirty(); });

    layout_->addWidget(buttonSetAsCurrent_,                  0, 0);
    layout_->addWidget(new QLabel("Resource Prefix Paths:"), 1, 0);
    layout_->addWidget(fieldResourcePrefixPaths_,            1, 1);
    layout_->addWidget(new QLabel("Resource Paths:"),        2, 0);
    layout_->addWidget(fieldResourcePaths_,                  2, 1);
    layout_->setRowStretch(3, 1);
}

bool ProjectDocument::DoLoad(const QString& fileName)
{
    project_->SetFileName(fileName);
    if (!project_->Load())
        return false;

    fieldResourcePrefixPaths_->setText(project_->GetResourcePrefixPaths());
    fieldResourcePaths_->setText(project_->GetResourcePaths());
    return true;
}

bool ProjectDocument::DoSave(const QString& fileName)
{
    project_->SetResourcePrefixPaths(fieldResourcePrefixPaths_->text());
    project_->SetResourcePaths(fieldResourcePaths_->text());

    project_->SetFileName(fileName);
    return project_->Save();
}

}
