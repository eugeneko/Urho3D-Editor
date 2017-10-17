#include "QtUI.h"
#include "QtUrhoHelpers.h"
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/IO/Log.h>
#include <QKeySequence>
#include <QMenuBar>
#include <QScrollBar>
#include <QPainter>
#include <QResizeEvent>

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

QWidget* GetInternalWidget(AbstractWidget* widget)
{
    return widget->GetInternalHandle<QWidget*>();
}

void SetInternalWidget(AbstractWidget* widget, QWidget* element)
{
    widget->SetInternalHandle(element);
}

}

//////////////////////////////////////////////////////////////////////////
bool QtDockDialog::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalWidget(content))
        return false;

    dock_->setWidget(GetInternalWidget(content));
    return true;
}

QtDockDialog::QtDockDialog(AbstractMainWindow& mainWindow)
    : AbstractDialog(mainWindow)
    , dock_(new QDockWidget())
{
    SetInternalWidget(this, dock_);
}

void QtDockDialog::SetName(const String& name)
{
    dock_->setWindowTitle(Cast(name));
}

//////////////////////////////////////////////////////////////////////////
QtDummyWidget::QtDummyWidget(AbstractMainWindow& mainWindow)
    : AbstractDummyWidget(mainWindow)
    , widget_(new QWidget())
{
    widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    SetInternalWidget(this, widget_);
}

//////////////////////////////////////////////////////////////////////////
QtScrollArea::QtScrollArea(AbstractMainWindow& mainWindow)
    : AbstractScrollArea(mainWindow)
    , scrollArea_(new QScrollArea())
{
    scrollArea_->setWidgetResizable(true);
    scrollArea_->installEventFilter(this);
    scrollArea_->verticalScrollBar()->installEventFilter(this);

    SetInternalWidget(this, scrollArea_);
}

void QtScrollArea::SetDynamicWidth(bool dynamicWidth)
{
    dynamicWidth_ = dynamicWidth;
    scrollArea_->setHorizontalScrollBarPolicy(dynamicWidth ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
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

bool QtScrollArea::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalWidget(content))
        return false;

    scrollArea_->setWidget(GetInternalWidget(content));
    return true;
}

//////////////////////////////////////////////////////////////////////////
QtLayout::QtLayout(AbstractMainWindow& mainWindow)
    : AbstractLayout(mainWindow)
    , widget_(new QWidget())
{
    layout_ = new QGridLayout(widget_);
    layout_->setVerticalSpacing(2);
    layout_->setContentsMargins(2, 2, 2, 2);
    SetInternalWidget(this, widget_);
}

bool QtLayout::DoSetCell(unsigned row, unsigned column, AbstractWidget* child)
{
    if (!GetInternalWidget(child))
        return false;

    layout_->addWidget(GetInternalWidget(child), row, column);
    return true;
}

bool QtLayout::DoSetRow(unsigned row, AbstractWidget* child)
{
    if (!GetInternalWidget(child))
        return false;

    layout_->addWidget(GetInternalWidget(child), row, 0, 1, -1);
    return true;
}

void QtLayout::DoRemoveChild(AbstractWidget* child)
{
    if (!GetInternalWidget(child))
        return;

    QWidget* widget = GetInternalWidget(child);
    layout_->removeWidget(widget);
    delete widget;
    SetInternalWidget(child, nullptr);
}

//////////////////////////////////////////////////////////////////////////
QtCollapsiblePanel::QtCollapsiblePanel(AbstractMainWindow& mainWindow)
    : AbstractCollapsiblePanel(mainWindow)
    , panel_(new QFrame())
{
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

    SetInternalWidget(this, panel_);
}

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

bool QtCollapsiblePanel::DoSetHeaderPrefix(AbstractWidget* header)
{
    if (!GetInternalWidget(header))
        return false;
    layout_->removeWidget(headerPrefix_);
    headerPrefix_ = GetInternalWidget(header);
    layout_->addWidget(headerPrefix_, 0, 1);
    UpdateHeaderHeight();
    UpdateSize();
    return true;
}

bool QtCollapsiblePanel::DoSetHeaderSuffix(AbstractWidget* header)
{
    if (!GetInternalWidget(header))
        return false;

    layout_->removeWidget(headerSuffix_);
    headerSuffix_ = GetInternalWidget(header);
    layout_->addWidget(headerSuffix_, 0, 3);
    UpdateHeaderHeight();
    UpdateSize();
    return true;
}

bool QtCollapsiblePanel::DoSetBody(AbstractWidget* body)
{
    if (!GetInternalWidget(body))
        return false;
    layout_->removeWidget(body_);
    body_ = GetInternalWidget(body);
    layout_->addWidget(body_, 1, 0, 1, -1);
    UpdateSize();
    return true;
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
    {
        body_->setVisible(expanded_);
        panel_->updateGeometry();
    }
}

//////////////////////////////////////////////////////////////////////////
QtButton::QtButton(AbstractMainWindow& mainWindow)
    : AbstractButton(mainWindow)
    , pushButton_(new QPushButton())
{
    pushButton_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    SetInternalWidget(this, pushButton_);
}

void QtButton::SetText(const String& text)
{
    pushButton_->setText(Cast(text));
}

//////////////////////////////////////////////////////////////////////////
QtText::QtText(AbstractMainWindow& mainWindow)
    : AbstractText(mainWindow)
    , label_(new QLabel())
{
    SetInternalWidget(this, label_);
}

void QtText::SetText(const String& text)
{
    label_->setText(Cast(text));
}

unsigned QtText::GetTextWidth() const
{
    return static_cast<unsigned>(label_->sizeHint().width());
}

//////////////////////////////////////////////////////////////////////////
QtLineEdit::QtLineEdit(AbstractMainWindow& mainWindow)
    : AbstractLineEdit(mainWindow)
    , lineEdit_(new QLineEdit())
{
    connect(lineEdit_, &QLineEdit::textEdited,
        [=]()
    {
        if (onTextEdited_)
            onTextEdited_();
    });

    connect(lineEdit_, &QLineEdit::returnPressed,
        [=]()
    {
        if (onTextFinished_)
            onTextFinished_();
    });

    SetInternalWidget(this, lineEdit_);
}

void QtLineEdit::SetText(const String& text)
{
    lineEdit_->setText(Cast(text));
}

const String& QtLineEdit::GetText() const
{
    cachedText_ = Cast(lineEdit_->text());
    return cachedText_;
}

//////////////////////////////////////////////////////////////////////////
QtCheckBox::QtCheckBox(AbstractMainWindow& mainWindow)
    : AbstractCheckBox(mainWindow)
    , checkBox_(new QCheckBox())
{
    SetInternalWidget(this, checkBox_);
}

void QtCheckBox::SetChecked(bool checked)
{
    checkBox_->setChecked(checked);
}

//////////////////////////////////////////////////////////////////////////
QtHierarchyListModel::QtHierarchyListModel(AbstractMainWindow& mainWindow)
    : rootItem_(mainWindow.GetContext())
{
}

void QtHierarchyListModel::InsertItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex)
{
    AbstractHierarchyListItem* parentItem = GetItem(parentIndex);

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

void QtHierarchyListModel::RemoveItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex, int hintRow)
{
    AbstractHierarchyListItem* parentItem = GetItem(parentIndex);
    const int objectIndex = item->GetIndex();
    if (objectIndex >= 0)
    {
        beginRemoveRows(parentIndex, objectIndex, objectIndex);
        parentItem->RemoveChild(objectIndex);
        endRemoveRows();
    }
}

QModelIndex QtHierarchyListModel::GetIndex(AbstractHierarchyListItem* item, QModelIndex hint)
{
    if (!item)
        return QModelIndex();
    if (hint.isValid() && static_cast<AbstractHierarchyListItem*>(hint.internalPointer()) == item)
        return hint;

    AbstractHierarchyListItem* parent = item->GetParent();
    const int childIndex = item->GetIndex();
    return index(childIndex, 0, GetIndex(parent));
}

AbstractHierarchyListItem* QtHierarchyListModel::GetItem(const QModelIndex& index) const
{
    if (index.isValid())
    {
        AbstractHierarchyListItem* item = static_cast<AbstractHierarchyListItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return const_cast<AbstractHierarchyListItem*>(&rootItem_);
}

QVariant QtHierarchyListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AbstractHierarchyListItem* item = GetItem(index);
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

    AbstractHierarchyListItem* parentItem = GetItem(parent);
    AbstractHierarchyListItem* childItem = static_cast<AbstractHierarchyListItem*>(parentItem->GetChild(row));
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QtHierarchyListModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractHierarchyListItem* childItem = GetItem(index);
    AbstractHierarchyListItem* parentItem = static_cast<AbstractHierarchyListItem*>(childItem->GetParent());

    if (childItem == &rootItem_)
        return QModelIndex();

    return createIndex(parentItem->GetIndex(), 0, parentItem);
}

int QtHierarchyListModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    AbstractHierarchyListItem* parentItem = GetItem(parent);

    return parentItem->GetNumChildren();
}

//////////////////////////////////////////////////////////////////////////
QtHierarchyList::QtHierarchyList(AbstractMainWindow& mainWindow)
    : AbstractHierarchyList(mainWindow)
    , treeView_(new QTreeView())
    , model_(new QtHierarchyListModel(mainWindow_))
{
    treeView_->header()->hide();
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView_->setDragDropMode(QAbstractItemView::DragDrop);
    treeView_->setDragEnabled(true);
    treeView_->setModel(model_.data());

    SetInternalWidget(this, treeView_);
}

void QtHierarchyList::AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent)
{
    const QModelIndex parentIndex = model_->GetIndex(parent);
    model_->RemoveItem(item, parentIndex);
    model_->InsertItem(item, parentIndex);
}

void QtHierarchyList::SelectItem(AbstractHierarchyListItem* item)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void QtHierarchyList::DeselectItem(AbstractHierarchyListItem* item)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

void QtHierarchyList::GetSelection(ItemVector& result)
{
    //throw std::logic_error("The method or operation is not implemented.");
}

//////////////////////////////////////////////////////////////////////////
QtUrhoRenderSurface::QtUrhoRenderSurface(Texture2D* renderTexture, Texture2D* depthTexture, Viewport* viewport, Image* image_,
    QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , renderTexture_(renderTexture)
    , depthTexture_(depthTexture)
    , viewport_(viewport)
    , image_(image_)
{
    renderTexture_->SetNumLevels(1);
    depthTexture_->SetNumLevels(1);
}

void QtUrhoRenderSurface::SetView(Scene* scene, Camera* camera)
{
    if (!renderTexture_ || !depthTexture_ || !viewport_)
        return;
    viewport_->SetScene(scene);
    viewport_->SetCamera(camera);
}

void QtUrhoRenderSurface::paintEvent(QPaintEvent* event)
{
    if (!renderTexture_ || !depthTexture_ || !viewport_)
        return;

    if (!renderTexture_->GetImage(*image_))
        return;

    const int imageWidth = static_cast<int>(image_->GetWidth());
    const int imageHeight = static_cast<int>(image_->GetHeight());
    const QSize imageSize(imageWidth, imageHeight);
    if (imageData_.size() != imageSize)
        imageData_ = QImage(imageSize, QImage::Format_RGBA8888);

    unsigned char* sourceData = image_->GetData();
    for (int y = 0; y < imageHeight; ++y)
    {
        const unsigned stride = imageWidth * 4;
        memcpy(imageData_.scanLine(y), sourceData, stride);
        sourceData += stride;
    }
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), imageData_);
}

void QtUrhoRenderSurface::resizeEvent(QResizeEvent* event)
{
    if (!renderTexture_ || !depthTexture_ || !viewport_)
        return;

    viewport_->UnsubscribeFromEvent(E_RENDERSURFACEUPDATE);
    viewport_->UnsubscribeFromEvent(E_ENDVIEWRENDER);

    const QSize size = event->size().expandedTo(QSize(1, 1));
    renderTexture_->SetSize(size.width(), size.height(), Graphics::GetRGBAFormat(), TEXTURE_RENDERTARGET);
    depthTexture_->SetSize(size.width(), size.height(), Graphics::GetDepthStencilFormat(), TEXTURE_DEPTHSTENCIL);

    RenderSurface* surface = renderTexture_->GetRenderSurface();
    surface->SetViewport(0, viewport_);
    surface->SetLinkedDepthStencil(depthTexture_->GetRenderSurface());
    surface->SetUpdateMode(SURFACE_UPDATEALWAYS);

    viewport_->SubscribeToEvent(E_RENDERSURFACEUPDATE,
        [=](StringHash eventType, VariantMap& eventData)
    {
        surface->QueueUpdate();
    });

    viewport_->SubscribeToEvent(E_ENDVIEWRENDER,
        [=](StringHash eventType, VariantMap& eventData)
    {
        update();
    });
}

//////////////////////////////////////////////////////////////////////////
QtView3D::QtView3D(AbstractMainWindow& mainWindow)
    : AbstractView3D(mainWindow)
    , renderTexture_(new Texture2D(context_))
    , depthTexture_(new Texture2D(context_))
    , viewport_(new Viewport(context_))
    , image_(new Image(context_))
{
    // Setup Qt
    widget_ = new QtUrhoRenderSurface(renderTexture_, depthTexture_, viewport_, image_);
    widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    SetInternalWidget(this, widget_);
}

void QtView3D::SetView(Scene* scene, Camera* camera)
{
    widget_->SetView(scene, camera);
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

AbstractMenu* QtMenu::AddMenu(const String& name)
{
    if (!menu_)
        return nullptr;
    QMenu* childMenu = menu_->addMenu(Cast(name));
    children_.push_back(QtMenu(host_, childMenu));
    return &children_.back();
}

AbstractMenu* QtMenu::AddAction(const String& name, const String& actionId)
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
}

QtMainWindow::~QtMainWindow()
{
    // Delete actions first
    for (const auto& item : actions_)
        delete item.second_;
}

AbstractDialog* QtMainWindow::AddDialog(DialogLocationHint hint /*= DialogLocationHint::Undocked*/)
{
    auto dialog = MakeShared<QtDockDialog>(*this);
    dialog->SetParent(nullptr);
    QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(GetInternalWidget(dialog));
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

AbstractMenu* QtMainWindow::AddMenu(const String& name)
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
