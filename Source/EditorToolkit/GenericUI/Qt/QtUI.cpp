#include "QtUI.h"
#include "QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <QKeySequence>
#include <QMenuBar>

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

QKeySequence Cast(const KeyBinding& binding)
{
    int key = CastKey(binding.GetKey());
    if (binding.GetShift() == ModifierState::Required)
        key += Qt::SHIFT;
    if (binding.GetCtrl() == ModifierState::Required)
        key += Qt::CTRL;
    if (binding.GetAlt() == ModifierState::Required)
        key += Qt::ALT;
    return QKeySequence(key);
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
QtHierarchyListModel::QtHierarchyListModel(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : rootItem_(mainWindow.GetContext())
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
QtHierarchyList::QtHierarchyList(AbstractMainWindow& mainWindow, GenericWidget* parent)
    : GenericHierarchyList(mainWindow, parent)
    , model_(mainWindow, this)
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
QtMenu::QtMenu(QtMainWindow* host, QMenu* menu)
    : host_(host)
    , menu_(menu)
{

}

QtMenu::QtMenu(QtMainWindow* host, QAction* action)
    : host_(host)
    , action_(action)
{

}

GenericMenu* QtMenu::AddMenu(const String& name)
{
    if (!menu_)
        return nullptr;
    QMenu* childMenu = menu_->addMenu(Cast(name));
    children_.push_back(QtMenu(host_, childMenu));
    return &children_.back();
}

GenericMenu* QtMenu::AddAction(const String& name, const String& actionId)
{
    if (!menu_)
        return nullptr;

    QAction* childAction = host_->FindAction(actionId);
    if (!childAction)
        return nullptr;

    childAction->setText(Cast(name));
    menu_->addAction(childAction);
    children_.push_back(QtMenu(host_, childAction));
    return &children_.back();
}

//////////////////////////////////////////////////////////////////////////
QtMainWindow::QtMainWindow(QApplication& application)
    : AbstractMainWindow()
    , context_(new Context)
    , application_(application)
    , urhoWidget_(*context_, nullptr)
{
    urhoWidget_.Initialize(Engine::ParseParameters(GetArguments()));
    setCentralWidget(&urhoWidget_);
    showMaximized();
}

QtMainWindow::~QtMainWindow()
{
    // Delete actions first
    for (const auto& item : actions_)
        delete item.second_;
}

GenericDialog* QtMainWindow::AddDialog(DialogLocationHint hint /*= DialogLocationHint::Undocked*/)
{
    auto widget = new QtDockDialog(*this, nullptr);
    addDockWidget(Cast(hint), widget);
    return widget;
}

void QtMainWindow::AddAction(const AbstractAction& actionDesc)
{
    QAction* action = new QAction(this);
    action->setText(Cast(actionDesc.text_));
    action->setShortcut(Cast(actionDesc.keyBinding_));
    connect(action, &QAction::triggered, this, actionDesc.actionCallback_);
    actions_[actionDesc.id_] = action;
}

GenericMenu* QtMainWindow::AddMenu(const String& name)
{
    QMenu* menu = menuBar()->addMenu(Cast(name));
    menus_.push_back(QtMenu(this, menu));
    return &menus_.back();
}

Context* QtMainWindow::GetContext()
{
    return context_;
}

AbstractInput* QtMainWindow::GetInput()
{
    return urhoWidget_.GetInput();
}

QAction* QtMainWindow::FindAction(const String& id) const
{
    QAction* action = nullptr;
    actions_.TryGetValue(id, action);
    return action;
}

}
