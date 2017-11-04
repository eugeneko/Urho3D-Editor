#include "QtUI.h"

#ifdef URHO3D_COMPILE_QT
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

Qt::DockWidgetArea Cast(DockLocation value)
{
    switch (value)
    {
    case DockLocation::Left:
        return Qt::LeftDockWidgetArea;
    case DockLocation::Right:
        return Qt::RightDockWidgetArea;
    case DockLocation::Bottom:
        return Qt::BottomDockWidgetArea;
    case DockLocation::Top:
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

void CreateChildrenMenu(const AbstractMenuItem& desc, QMenu* menu)
{

}

}

//////////////////////////////////////////////////////////////////////////
QtDock::QtDock(AbstractMainWindow* mainWindow)
    : AbstractDock(mainWindow)
    , dock_(new QDockWidget())
    , widget_(new QWidgetHint())
{
    dock_->setWidget(widget_);

    SetInternalWidget(this, dock_);
}

void QtDock::SetSizeHint(const IntVector2& sizeHint)
{
    widget_->setSizeHint(QSize(sizeHint.x_, sizeHint.y_));
    widget_->resize(sizeHint.x_, sizeHint.y_);
}

bool QtDock::DoSetContent(AbstractWidget* content)
{
    if (!GetInternalWidget(content))
        return false;

    QWidget* widget = GetInternalWidget(content);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    widget_->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget);
    return true;
}

void QtDock::SetName(const String& name)
{
    dock_->setWindowTitle(Cast(name));
}

//////////////////////////////////////////////////////////////////////////
QtDummyWidget::QtDummyWidget(AbstractMainWindow* mainWindow)
    : AbstractDummyWidget(mainWindow)
    , widget_(new QWidget())
{
    widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    SetInternalWidget(this, widget_);
}

//////////////////////////////////////////////////////////////////////////
QtScrollArea::QtScrollArea(AbstractMainWindow* mainWindow)
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
QtLayout::QtLayout(AbstractMainWindow* mainWindow)
    : AbstractLayout(mainWindow)
    , widget_(new QWidget())
{
    layout_ = new QGridLayout(widget_);
    layout_->setVerticalSpacing(2);
    layout_->setContentsMargins(2, 2, 2, 2);
    widget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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
QtCollapsiblePanel::QtCollapsiblePanel(AbstractMainWindow* mainWindow)
    : AbstractCollapsiblePanel(mainWindow)
    , panel_(new QFrame())
{
    panel_->setFrameShape(QFrame::Box);
    panel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    layout_ = new QGridLayout(panel_);
    layout_->setVerticalSpacing(0);
    layout_->setContentsMargins(0, 0, 0, 0);

    toggleButton_ = new QToolButton();
    toggleButton_->setStyleSheet("QToolButton {border: none;}");
    toggleButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
QtWidgetStack::QtWidgetStack(AbstractMainWindow* mainWindow)
    : AbstractWidgetStackT<QWidget>(mainWindow)
    , stack_(new QStackedWidget)
{
    SetInternalWidget(this, stack_);
}

void QtWidgetStack::DoAddChild(QWidget* child)
{
    stack_->addWidget(child);
}

void QtWidgetStack::DoRemoveChild(QWidget* child)
{
    stack_->removeWidget(child);
}

void QtWidgetStack::DoSelectChild(QWidget* child)
{
    if (child)
        stack_->setCurrentWidget(child);
    else
        stack_->setCurrentIndex(-1);
}

//////////////////////////////////////////////////////////////////////////
QtButton::QtButton(AbstractMainWindow* mainWindow)
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
QtText::QtText(AbstractMainWindow* mainWindow)
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
QtLineEdit::QtLineEdit(AbstractMainWindow* mainWindow)
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

String QtLineEdit::GetText() const
{
    return Cast(lineEdit_->text());
}

//////////////////////////////////////////////////////////////////////////
QtCheckBox::QtCheckBox(AbstractMainWindow* mainWindow)
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
QtHierarchyListModel::QtHierarchyListModel(AbstractMainWindow* mainWindow)
    : rootItem_(mainWindow->GetContext())
{
}

void QtHierarchyListModel::InsertItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex, int row)
{
    AbstractHierarchyListItem* parentItem = GetItem(parentIndex);
    const int numChildren = static_cast<int>(parentItem->GetNumChildren());
    row = Clamp(row, 0, numChildren);

    beginInsertRows(QModelIndex(), row, row);
    parentItem->InsertChild(item, static_cast<unsigned>(row));
    endInsertRows();
}

void QtHierarchyListModel::RemoveItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex, int hintRow)
{
    AbstractHierarchyListItem* parentItem = GetItem(parentIndex);
    const int objectIndex = parentItem->FindChild(item);
    if (objectIndex >= 0)
    {
        beginRemoveRows(parentIndex, objectIndex, objectIndex);
        parentItem->RemoveChild(objectIndex);
        endRemoveRows();
    }
}

QModelIndex QtHierarchyListModel::GetIndex(AbstractHierarchyListItem* item, const QModelIndex& hint)
{
    if (!item || !item->GetParent())
        return QModelIndex();
    if (hint.isValid() && static_cast<AbstractHierarchyListItem*>(hint.internalPointer()) == item)
        return hint;

    AbstractHierarchyListItem* parent = item->GetParent();
    return index(item->GetIndex(), 0, GetIndex(parent, hint));
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
QtHierarchyList::QtHierarchyList(AbstractMainWindow* mainWindow)
    : AbstractHierarchyList(mainWindow)
    , treeView_(new QTreeView())
    , model_(new QtHierarchyListModel(mainWindow_))
{
    treeView_->header()->hide();
    treeView_->setDragDropMode(QAbstractItemView::DragDrop);
    treeView_->setDragEnabled(true);
    treeView_->setModel(model_.data());

    connect(treeView_, &QTreeView::clicked,
        [=](const QModelIndex& index)
    {
        if (AbstractHierarchyListItem* item = model_->GetItem(index))
        {
            if (onItemClicked_)
                onItemClicked_(item);
        }
    });

    connect(treeView_, &QTreeView::doubleClicked,
        [=](const QModelIndex& index)
    {
        if (AbstractHierarchyListItem* item = model_->GetItem(index))
        {
            if (onItemDoubleClicked_)
                onItemDoubleClicked_(item);
        }
    });

    SetInternalWidget(this, treeView_);
}

void QtHierarchyList::SetMultiselect(bool multiselect)
{
    treeView_->setSelectionMode(multiselect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
}

void QtHierarchyList::AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent)
{
    const QModelIndex parentIndex = model_->GetIndex(parent);
    model_->RemoveItem(item, parentIndex);
    model_->InsertItem(item, parentIndex);
    if (!parent)
        treeView_->expand(model_->GetIndex(item));
}

void QtHierarchyList::RemoveItem(AbstractHierarchyListItem* item)
{
    const QModelIndex parentIndex = model_->GetIndex(item->GetParent());
    model_->RemoveItem(item, parentIndex);
}

void QtHierarchyList::RemoveAllItems()
{
    // #TODO Reset more gracefully
    model_.reset(new QtHierarchyListModel(mainWindow_));
    treeView_->setModel(model_.data());
}

void QtHierarchyList::SelectItem(AbstractHierarchyListItem* item)
{
    QItemSelectionModel* selectionModel = treeView_->selectionModel();
    const QModelIndex itemIndex = model_->GetIndex(item);
    if (itemIndex.isValid())
    {
        selectionModel->select(itemIndex, QItemSelectionModel::Select);
        treeView_->scrollTo(itemIndex);
    }
}

void QtHierarchyList::DeselectItem(AbstractHierarchyListItem* item)
{
    QItemSelectionModel* selectionModel = treeView_->selectionModel();
    const QModelIndex itemIndex = model_->GetIndex(item);
    if (itemIndex.isValid())
        selectionModel->select(itemIndex, QItemSelectionModel::Deselect);
}

void QtHierarchyList::ExpandItem(AbstractHierarchyListItem* item)
{
    QModelIndex itemIndex = model_->GetIndex(item);
    while (itemIndex.isValid())
    {
        treeView_->expand(itemIndex);
        itemIndex = model_->parent(itemIndex);
    }
}

void QtHierarchyList::GetSelection(ItemVector& result)
{
    QItemSelectionModel* selectionModel = treeView_->selectionModel();
    QModelIndexList selection = selectionModel->selectedIndexes();
    for (const QModelIndex& index : selection)
        if (AbstractHierarchyListItem* item = model_->GetItem(index))
            result.Push(item);
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

void QtUrhoRenderSurface::SetAutoUpdate(bool autoUpdate)
{
    autoUpdate_ = autoUpdate;
}

void QtUrhoRenderSurface::paintEvent(QPaintEvent* event)
{
    if (!renderTexture_ || !depthTexture_ || !viewport_)
        return;

    // Get image if dirty
    if (imageDirty_ && renderTexture_->GetImage(*image_))
    {
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
    }

    // Draw image
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), imageData_);
}

void QtUrhoRenderSurface::resizeEvent(QResizeEvent* event)
{
    if (!renderTexture_ || !depthTexture_ || !viewport_)
        return;

    imageDirty_ = false;
    viewport_->UnsubscribeFromEvent(E_RENDERSURFACEUPDATE);
    viewport_->UnsubscribeFromEvent(E_ENDVIEWRENDER);

    const QSize size = event->size().expandedTo(QSize(1, 1));
    renderTexture_->SetSize(size.width(), size.height(), Graphics::GetRGBAFormat(), TEXTURE_RENDERTARGET);
    depthTexture_->SetSize(size.width(), size.height(), Graphics::GetDepthStencilFormat(), TEXTURE_DEPTHSTENCIL);

    RenderSurface* surface = renderTexture_->GetRenderSurface();
    surface->SetViewport(0, viewport_);
    surface->SetLinkedDepthStencil(depthTexture_->GetRenderSurface());
    surface->SetUpdateMode(SURFACE_MANUALUPDATE);

    viewport_->SubscribeToEvent(E_RENDERSURFACEUPDATE,
        [=](StringHash eventType, VariantMap& eventData)
    {
        if (RenderSurface* surface = renderTexture_->GetRenderSurface())
        {
            if (autoUpdate_)
                surface->QueueUpdate();
        }
    });

    viewport_->SubscribeToEvent(E_ENDVIEWRENDER,
        [=](StringHash eventType, VariantMap& eventData)
    {
        if (eventData[EndViewRender::P_TEXTURE].GetPtr() == renderTexture_)
        {
            imageDirty_ = true;
            update();
        }
    });

    surface->QueueUpdate();
}

//////////////////////////////////////////////////////////////////////////
QtView3D::QtView3D(AbstractMainWindow* mainWindow)
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

void QtView3D::SetAutoUpdate(bool autoUpdate)
{
    widget_->SetAutoUpdate(autoUpdate);
}

void QtView3D::UpdateView()
{
    widget_->update();
}

//////////////////////////////////////////////////////////////////////////
QtContextMenu::QtContextMenu(Context* context)
    : AbstractContextMenu(context)
{
}

QtContextMenu::~QtContextMenu()
{
}

void QtContextMenu::Show()
{
    exec(QCursor::pos());
}

//////////////////////////////////////////////////////////////////////////
QtMainWindow::QtMainWindow(QApplication& application)
    : AbstractMainWindow()
    , context_(new Context)
    , application_(application)
    , centralLayout_(&centralWidget_)
    , urhoWidget_(*context_, nullptr)
{
    urhoWidget_.Initialize(Engine::ParseParameters(GetArguments()));
    setCentralWidget(&centralWidget_);
    centralLayout_.addWidget(&documentsBar_, 0, 0);
    centralLayout_.addWidget(&urhoWidget_, 1, 0);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    documentsBar_.setExpanding(false);

    connect(&documentsBar_, &QTabBar::tabMoved,
        [=](int from, int to)
    {
        documents_.move(from, to);
    });

    connect(&documentsBar_, &QTabBar::currentChanged,
        [=](int index)
    {
        if (index >= 0 && index < documents_.size())
        {
            if (onCurrentDocumentChanged_)
                onCurrentDocumentChanged_(documents_[index]);
        }
    });
}

QtMainWindow::~QtMainWindow()
{
}

AbstractDock* QtMainWindow::AddDock(DockLocation hint, const IntVector2& sizeHint)
{
    auto dialog = MakeShared<QtDock>(this);
    dialog->SetSizeHint(sizeHint);
    dialog->SetParent(nullptr);
    QDockWidget* dockWidget = dynamic_cast<QDockWidget*>(GetInternalWidget(dialog));
    addDockWidget(Cast(hint), dockWidget);
    dialogs_.Push(dialog);
    return dialog;
}

void QtMainWindow::CreateMainMenu(const AbstractMenuItem& desc)
{
    for (const AbstractMenuItem& child : desc.children_)
    {
        if (child.IsPopupMenu())
        {
            QMenu* childPopup = menuBar()->addMenu(Cast(child.text_));
            SetupMenu(childPopup, child);
        }
        else if (child.IsSeparator())
        {
            menuBar()->addSeparator();
        }
        else
        {
            QAction* childAction = menuBar()->addAction(Cast(child.text_));
            SetupAction(childAction, child);
        }
    }
}

SharedPtr<AbstractContextMenu> QtMainWindow::CreateContextMenu(const AbstractMenuItem& desc)
{
    auto contextMenu = MakeShared<QtContextMenu>(context_);
    SetupMenu(contextMenu, desc);
    return contextMenu;
}

void QtMainWindow::InsertDocument(Object* document, const String& title, unsigned index)
{
    const int clampedIndex = Clamp(static_cast<int>(index), 0, documentsBar_.count());
    documents_.insert(clampedIndex, SharedPtr<Object>(document));
    const int tabIndex = documentsBar_.insertTab(clampedIndex, Cast(title));
    assert(tabIndex == clampedIndex);
}

void QtMainWindow::SelectDocument(Object* document)
{
    const int tab = documents_.indexOf(SharedPtr<Object>(document));
    documentsBar_.setCurrentIndex(tab);
}

PODVector<Object*> QtMainWindow::GetDocuments() const
{
    PODVector<Object*> result;
    for (Object* object : documents_)
        result.Push(object);
    return result;
}

Context* QtMainWindow::GetContext()
{
    return context_;
}

AbstractInput* QtMainWindow::GetInput()
{
    return urhoWidget_.GetInput();
}

void QtMainWindow::SetupAction(QAction* action, const AbstractMenuItem& desc)
{
    action->setShortcut(Cast(desc.hotkey_));

    auto onActivated = desc.action_ ? desc.action_->onActivated_ : nullptr;
    connect(action, &QAction::triggered,
        [=]()
    {
        if (onActivated)
            onActivated();
    });
}

void QtMainWindow::SetupMenu(QMenu* menu, const AbstractMenuItem& desc)
{
    Vector<Pair<QAction*, AbstractMenuAction*>> actions;
    for (const AbstractMenuItem& child : desc.children_)
    {
        if (child.IsPopupMenu())
        {
            QMenu* childPopup = menu->addMenu(Cast(child.text_));
            SetupMenu(childPopup, child);
        }
        else if (child.IsSeparator())
        {
            menu->addSeparator();
        }
        else
        {
            QAction* childAction = menu->addAction(Cast(child.text_));
            SetupAction(childAction, child);
            actions.Push(MakePair(childAction, child.action_));
        }
    }

    connect(menu, &QMenu::aboutToShow, [actions]()
    {
        for (const auto& item : actions)
        {
            QAction* qtAction = item.first_;
            AbstractMenuAction* actionDesc = item.second_;
            if (actionDesc && actionDesc->onUpdateText_)
            {
                String text = Cast(qtAction->text());
                actionDesc->onUpdateText_(text);
                qtAction->setText(Cast(text));
            }
        }
    });
}

}

#endif
