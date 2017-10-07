#include "QtUI.h"
#include "QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>

namespace Urho3D
{

namespace
{

static int argcStub = 0;
static char* argvStub[] = { nullptr };

Qt::DockWidgetArea Cast(DialogLocationHint value)
{
    switch (value)
    {
    case DialogLocationHint::DockLeft:
        return Qt::LeftDockWidgetArea;
    case DialogLocationHint::DockRight:
        return Qt::RightDockWidgetArea;
    case DialogLocationHint::DockBottom:
        return Qt::BottomDockWidgetArea;
    case DialogLocationHint::DockTop:
        return Qt::TopDockWidgetArea;
    default:
        return Qt::NoDockWidgetArea;
    }
}

}

void QtDockDialog::SetBodyWidget(GenericWidget* widget)
{
    if (QWidget* qwidget = dynamic_cast<QWidget*>(widget))
        setWidget(qwidget);
}

void QtDockDialog::SetName(const String& name)
{
    setWindowTitle(Cast(name));
}

//////////////////////////////////////////////////////////////////////////
QtHierarchyListModel::QtHierarchyListModel(AbstractUI& ui, GenericWidget* parent)
    : rootItem_(ui.GetContext())
{
}

void QtHierarchyListModel::InsertItem(GenericHierarchyListItem* item, const QModelIndex& parentIndex)
{
    GenericHierarchyListItem* parentItem = GetItem(parentIndex);

    if (!parentIndex.isValid())
    {
        const int childIndex = static_cast<int>(parentItem->GetNumChildren());
        beginInsertRows(QModelIndex(), childIndex, childIndex);
        parentItem->InsertChild(item, M_MAX_UNSIGNED);
        endInsertRows();
    }
    else
    {
        const int childIndex = item->GetIndex();
        if (childIndex >= 0)
        {
            beginInsertRows(parentIndex, childIndex, childIndex);
            parentItem->InsertChild(item, childIndex);
            endInsertRows();
        }
    }
}

void QtHierarchyListModel::RemoveItem(GenericHierarchyListItem* item, const QModelIndex& parentIndex, int hintRow)
{
    GenericHierarchyListItem* parentItem = GetItem(parentIndex);
    const int objectIndex = item->GetIndex();
    if (objectIndex >= 0)
    {
        beginRemoveRows(parentIndex, objectIndex, objectIndex);
        parentItem->RemoveChild(objectIndex);
        endRemoveRows();
    }
}

QModelIndex QtHierarchyListModel::GetIndex(GenericHierarchyListItem* item, QModelIndex hint)
{
    if (!item)
        return QModelIndex();
    if (hint.isValid() && static_cast<GenericHierarchyListItem*>(hint.internalPointer()) == item)
        return hint;

    GenericHierarchyListItem* parent = item->GetParent();
    const int childIndex = item->GetIndex();
    return index(childIndex, 0, GetIndex(parent));
}

GenericHierarchyListItem* QtHierarchyListModel::GetItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        GenericHierarchyListItem* item = static_cast<GenericHierarchyListItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return const_cast<GenericHierarchyListItem*>(&rootItem_);
}

QVariant QtHierarchyListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    GenericHierarchyListItem* item = GetItem(index);
    switch (role)
    {
    case Qt::DisplayRole:
        return Cast(item->GetText());
//     case Qt::TextColorRole:
//         return spec_.GetObjectColor(item->GetObject());
    default:
        return QVariant();
    }
}

QVariant QtHierarchyListModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    return QVariant();
}

QModelIndex QtHierarchyListModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    GenericHierarchyListItem* parentItem = GetItem(parent);
    GenericHierarchyListItem* childItem = static_cast<GenericHierarchyListItem*>(parentItem->GetChild(row));
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QtHierarchyListModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    GenericHierarchyListItem* childItem = GetItem(index);
    GenericHierarchyListItem* parentItem = static_cast<GenericHierarchyListItem*>(childItem->GetParent());

    if (childItem == &rootItem_)
        return QModelIndex();

    return createIndex(parentItem->GetIndex(), 0, parentItem);
}

int QtHierarchyListModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    GenericHierarchyListItem* parentItem = GetItem(parent);

    return parentItem->GetNumChildren();
}

//////////////////////////////////////////////////////////////////////////
QtHierarchyList::QtHierarchyList(AbstractUI& ui, GenericWidget* parent)
    : GenericHierarchyList(ui, parent)
    , model_(ui, this)
{
    header()->hide();
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setModel(&model_);
}

void QtHierarchyList::AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent)
{
    const QModelIndex parentIndex = model_.GetIndex(parent);
    model_.RemoveItem(item, parentIndex);
    model_.InsertItem(item, parentIndex);
}

void QtHierarchyList::SelectItem(GenericHierarchyListItem* item)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void QtHierarchyList::DeselectItem(GenericHierarchyListItem* item)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void QtHierarchyList::GetSelection(ItemVector& result)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

//////////////////////////////////////////////////////////////////////////
QtMainWindow::QtMainWindow(QApplication& application)
    : GenericMainWindow()
    , context_(new Context)
    , application_(application)
    , mainWindow_()
    , urhoWidget_(*context_, nullptr)
    , ui_(*this)
{
    urhoWidget_.Initialize(Engine::ParseParameters(GetArguments()));
    mainWindow_.setCentralWidget(&urhoWidget_);
    mainWindow_.showMaximized();
}

GenericDialog* QtMainWindow::AddDialog(DialogLocationHint hint /*= DialogLocationHint::Undocked*/)
{
    auto widget = new QtDockDialog(ui_, nullptr);
    mainWindow_.addDockWidget(Cast(hint), widget);
    return widget;
}

//////////////////////////////////////////////////////////////////////////
GenericWidget* QtUI::CreateWidget(StringHash type, GenericWidget* parent)
{
    GenericWidget* widget = nullptr;
    if (type == GenericHierarchyList::GetTypeStatic())
        widget = new QtHierarchyList(*this, parent);
    return widget;
}

Context* QtUI::GetContext()
{
    return mainWindow_.GetContext();
}

GenericMainWindow* QtUI::GetMainWindow()
{
    return &mainWindow_;
}

}
