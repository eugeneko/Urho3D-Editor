#include "QtUI.h"
#include "QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <QKeySequence>
#include <QMenuBar>
#include <QScrollBar>

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

bool QtDockDialog::SetContent(GenericWidget* content)
{
    if (auto contentWidget = dynamic_cast<QtWidget*>(content))
    {
        dock_->setWidget(contentWidget->CreateWidget());
        return true;
    }
    return false;
}

void QtDockDialog::SetName(const String& name)
{
    dock_->setWindowTitle(Cast(name));
}

QWidget* QtDockDialog::CreateWidget()
{
    dock_ = new QDockWidget();
    return dock_;
}

//////////////////////////////////////////////////////////////////////////
QWidget* QtDummyWidget::CreateWidget()
{
    QWidget* widget = new QWidget;
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    return widget;
}

//////////////////////////////////////////////////////////////////////////
void QtScrollArea::SetDynamicWidth(bool dynamicWidth)
{
    dynamicWidth_ = dynamicWidth;
    scrollArea_->setHorizontalScrollBarPolicy(dynamicWidth ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
}

QWidget* QtScrollArea::CreateWidget()
{
    scrollArea_ = new QScrollArea();
    scrollArea_->setWidgetResizable(true);
    scrollArea_->installEventFilter(this);
    scrollArea_->verticalScrollBar()->installEventFilter(this);
    return scrollArea_;
}

bool QtScrollArea::eventFilter(QObject* watched, QEvent* event)
{
    if (dynamicWidth_ && (watched == scrollArea_ || watched == scrollArea_->verticalScrollBar()) && event->type() == QEvent::Resize)
        UpdateContentSize();
    return false;
}

void QtScrollArea::UpdateContentSize()
{
    if (QWidget* content = scrollArea_->widget())
        content->setMaximumWidth(scrollArea_->width() - scrollArea_->verticalScrollBar()->width());
}

bool QtScrollArea::SetContent(GenericWidget* content)
{
    if (auto contentWidget = dynamic_cast<QtWidget*>(content))
    {
        QWidget* widget = contentWidget->CreateWidget();
        scrollArea_->setWidget(widget);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
QWidget* QtLayout::CreateWidget()
{
    widget_ = new QWidget();
    layout_ = new QGridLayout(widget_);
    layout_->setVerticalSpacing(2);
    layout_->setContentsMargins(2, 2, 2, 2);
    return widget_;
}

bool QtLayout::SetCellWidget(unsigned row, unsigned column, GenericWidget* child)
{
    if (auto childWidget = dynamic_cast<QtWidget*>(child))
    {
        layout_->addWidget(childWidget->CreateWidget(), row, column);
        return true;
    }
    return false;
}

bool QtLayout::SetRowWidget(unsigned row, GenericWidget* child)
{
    if (auto childWidget = dynamic_cast<QtWidget*>(child))
    {
        layout_->addWidget(childWidget->CreateWidget(), row, 0, 1, -1);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
void QtCollapsiblePanel::SetHeaderText(const String& text)
{
    headerText_->setText(Cast(text));
}

void QtCollapsiblePanel::SetExpanded(bool expanded)
{
    toggleButton_->setChecked(expanded);
    expanded_ = expanded;
    UpdateSize();
}

QWidget* QtCollapsiblePanel::CreateWidget()
{
    panel_ = new QFrame();
    panel_->setFrameShape(QFrame::Box);

    layout_ = new QGridLayout(panel_);
    layout_->setVerticalSpacing(0);
    layout_->setContentsMargins(0, 0, 0, 0);

    toggleButton_ = new QToolButton();
    toggleButton_->setStyleSheet("QToolButton {border: none;}");
    toggleButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton_->setArrowType(Qt::ArrowType::RightArrow);
    toggleButton_->setCheckable(true);
    toggleButton_->setChecked(expanded_);
    layout_->addWidget(toggleButton_, 0, 0);

    headerText_ = new QLabel();
    layout_->addWidget(headerText_, 0, 2, Qt::AlignLeft);
    layout_->setColumnStretch(2, 1);

    connect(toggleButton_, &QToolButton::clicked, this, &QtCollapsiblePanel::SetExpanded);
    UpdateHeaderHeight();
    UpdateSize();
    return panel_;
}

bool QtCollapsiblePanel::SetHeaderPrefix(GenericWidget* header)
{
    if (auto headerImpl = dynamic_cast<QtWidget*>(header))
    {
        QWidget* newHeader = headerImpl->CreateWidget();
        layout_->removeWidget(headerPrefix_);
        layout_->addWidget(newHeader, 0, 1);
        headerPrefix_ = newHeader;
        UpdateHeaderHeight();
        UpdateSize();
        return true;
    }
    return false;
}

bool QtCollapsiblePanel::SetHeaderSuffix(GenericWidget* header)
{
    if (auto headerImpl = dynamic_cast<QtWidget*>(header))
    {
        QWidget* newHeader = headerImpl->CreateWidget();
        layout_->removeWidget(headerSuffix_);
        layout_->addWidget(newHeader, 0, 3);
        headerSuffix_ = newHeader;
        UpdateHeaderHeight();
        UpdateSize();
        return true;
    }
    return false;
}

bool QtCollapsiblePanel::SetBody(GenericWidget* body)
{
    if (auto bodyWidget = dynamic_cast<QtWidget*>(body))
    {
        QWidget* newBody = bodyWidget->CreateWidget();
        layout_->removeWidget(body_);
        layout_->addWidget(newBody, 1, 0, 1, -1);
        body_ = newBody;
        UpdateSize();
        return true;
    }
    return false;
}

void QtCollapsiblePanel::UpdateHeaderHeight()
{
    headerHeight_ = toggleButton_->sizeHint().height();
    if (headerPrefix_)
        headerHeight_ = Max(headerHeight_, headerPrefix_->sizeHint().height());
}

void QtCollapsiblePanel::UpdateSize()
{
    toggleButton_->setArrowType(expanded_ ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
//     int bodyHeight = body_ ? body_->sizeHint().height() : 0;
//     panel_->setMinimumHeight(expanded_ ? headerHeight_ + bodyHeight : headerHeight_);
//     panel_->setMaximumHeight(expanded_ ? headerHeight_ + bodyHeight : headerHeight_);
    if (body_)
        body_->setVisible(expanded_);
}

//////////////////////////////////////////////////////////////////////////
AbstractButton& QtButton::SetText(const String& text)
{
    pushButton_->setText(Cast(text));
    return *this;
}

QWidget* QtButton::CreateWidget()
{
    pushButton_ = new QPushButton();
    pushButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return pushButton_;
}

//////////////////////////////////////////////////////////////////////////
AbstractText& QtText::SetText(const String& text)
{
    label_->setText(Cast(text));
    return *this;
}

QWidget* QtText::CreateWidget()
{
    label_ = new QLabel();
    return label_;
}

//////////////////////////////////////////////////////////////////////////
AbstractLineEdit& QtLineEdit::SetText(const String& text)
{
    lineEdit_->setText(Cast(text));
    return *this;
}

QWidget* QtLineEdit::CreateWidget()
{
    lineEdit_ = new QLineEdit();
    return lineEdit_;
}

//////////////////////////////////////////////////////////////////////////
AbstractCheckBox& QtCheckBox::SetChecked(bool checked)
{
    checkBox_->setChecked(checked);
    return *this;
}

QWidget* QtCheckBox::CreateWidget()
{
    checkBox_ = new QCheckBox();
    return checkBox_;
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
    auto dialog = MakeShared<QtDockDialog>(*this, nullptr);
    QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(dialog->CreateWidget());
    addDockWidget(Cast(hint), dockWidget);
    dialogs_.Push(dialog);
    return dialog;
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
