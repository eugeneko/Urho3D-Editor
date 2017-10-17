#pragma once

#include "../GenericUI.h"
#include "../UrhoUI.h"
#include "QtUrhoWidget.h"
#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QAction>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QFrame>
#include <QImage>
#include <QToolButton>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/RenderSurface.h>
#include <Urho3D/Resource/Image.h>

namespace Urho3D
{

class QtMainWindow;

class QtDockDialog : public AbstractDialog
{

public:
    QtDockDialog(AbstractMainWindow& mainWindow);
    void SetName(const String& name) override;


private:
    bool DoSetContent(AbstractWidget* content) override;

private:
    QDockWidget* dock_ = nullptr;
};

class QtDummyWidget : public AbstractDummyWidget
{
public:
    QtDummyWidget(AbstractMainWindow& mainWindow);

private:
    QWidget* widget_ = nullptr;

};

class QtScrollArea : public QObject, public AbstractScrollArea
{
public:
    QtScrollArea(AbstractMainWindow& mainWindow);

    void SetDynamicWidth(bool dynamicWidth) override;



private:
    bool DoSetContent(AbstractWidget* content) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    void UpdateContentSize();

private:
    bool dynamicWidth_ = false;
    QScrollArea* scrollArea_ = nullptr;

};

class QtLayout : public AbstractLayout
{
public:
    QtLayout(AbstractMainWindow& mainWindow);



private:
    bool DoSetCell(unsigned row, unsigned column, AbstractWidget* child) override;
    bool DoSetRow(unsigned row, AbstractWidget* child) override;
    void DoRemoveChild(AbstractWidget* child) override;

private:
    QWidget* widget_ = nullptr;
    QGridLayout* layout_ = nullptr;

};

class QtCollapsiblePanel : public QObject, public AbstractCollapsiblePanel
{
    Q_OBJECT

public:
    QtCollapsiblePanel(AbstractMainWindow& mainWindow);

    void SetHeaderText(const String& text) override;
    void SetExpanded(bool expanded) override;


private:
    bool DoSetHeaderPrefix(AbstractWidget* header) override;
    bool DoSetHeaderSuffix(AbstractWidget* header) override;
    bool DoSetBody(AbstractWidget* body) override;

    void UpdateHeaderHeight();
    void UpdateSize();

private:
    /// Panel widget.
    QFrame* panel_ = nullptr;
    /// Main layout.
    QGridLayout* layout_ = nullptr;
    /// Button.
    QToolButton* toggleButton_ = nullptr;
    /// Header prefix widget.
    QWidget* headerPrefix_ = nullptr;
    /// Header title widget.
    QLabel* headerText_ = nullptr;
    /// Header suffix widget.
    QWidget* headerSuffix_ = nullptr;
    /// Body widget.
    QWidget* body_ = nullptr;
    /// Is expanded?
    bool expanded_ = false;
    /// Height of the header.
    int headerHeight_ = 0;
    /// Height of the body.
    int bodyHeight_ = 0;

};

class QtButton : public AbstractButton
{
public:
    QtButton(AbstractMainWindow& mainWindow);
    void SetText(const String& text) override;


private:
    QPushButton* pushButton_ = nullptr;

};

class QtText : public AbstractText
{
    URHO3D_OBJECT(AbstractText, AbstractWidget);

public:
    QtText(AbstractMainWindow& mainWindow);
    void SetText(const String& text) override;
    unsigned GetTextWidth() const override;

private:
    QLabel* label_ = nullptr;

};

class QtLineEdit : public QObject, public AbstractLineEdit
{
    Q_OBJECT
    URHO3D_OBJECT(AbstractLineEdit, AbstractWidget);

public:
    QtLineEdit(AbstractMainWindow& mainWindow);
    void SetText(const String& text) override;
    const String& GetText() const override;

private:
    QLineEdit* lineEdit_ = nullptr;
    mutable String cachedText_;

};

class QtCheckBox : public AbstractCheckBox
{
    URHO3D_OBJECT(AbstractCheckBox, QtCheckBox);

public:
    QtCheckBox(AbstractMainWindow& mainWindow);
    void SetChecked(bool checked) override;

private:
    QCheckBox* checkBox_ = nullptr;
};

class QtHierarchyListModel : public QAbstractItemModel
{
public:
    QtHierarchyListModel(AbstractMainWindow& mainWindow);

    void InsertItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex);
    void RemoveItem(AbstractHierarchyListItem* item, const QModelIndex& parentIndex, int hintRow = -1);

    QModelIndex GetIndex(AbstractHierarchyListItem* item, QModelIndex hint = QModelIndex());
    AbstractHierarchyListItem* GetItem(const QModelIndex& index) const;
private:

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }

    AbstractHierarchyListItem rootItem_;

};

class QtHierarchyList : public AbstractHierarchyList
{
public:
    QtHierarchyList(AbstractMainWindow& mainWindow);

    void AddItem(AbstractHierarchyListItem* item, unsigned index, AbstractHierarchyListItem* parent) override;
    void SelectItem(AbstractHierarchyListItem* item) override;
    void DeselectItem(AbstractHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

private:

    QTreeView* treeView_ = nullptr;
    QScopedPointer<QtHierarchyListModel> model_;

};

class QtUrhoRenderSurface : public QWidget
{
    Q_OBJECT

public:
    QtUrhoRenderSurface(Texture2D* renderTexture, Texture2D* depthTexture, Viewport* viewport, Image* image_, QWidget* parent = nullptr);
    /// Set the content of the view.
    void SetView(Scene* scene, Camera* camera);
    void SetAutoUpdate(bool autoUpdate);

private:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    WeakPtr<Texture2D> renderTexture_;
    WeakPtr<Texture2D> depthTexture_;
    WeakPtr<Viewport> viewport_;
    WeakPtr<Image> image_;
    QImage imageData_;
    bool autoUpdate_ = true;
    bool imageDirty_ = false;

};

class QtView3D : public AbstractView3D
{
public:
    QtView3D(AbstractMainWindow& mainWindow);
    void SetView(Scene* scene, Camera* camera) override;
    void SetAutoUpdate(bool autoUpdate) override;
    void UpdateView() override;

private:
    QtUrhoRenderSurface* widget_ = nullptr;
    SharedPtr<Texture2D> renderTexture_;
    SharedPtr<Texture2D> depthTexture_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<Image> image_;
};

class QtMenu : public AbstractMenu
{
public:
    QtMenu(QtMainWindow* host, QMenu* menu);
    QtMenu(QtMainWindow* host, QAction* action);
    AbstractMenu* AddMenu(const String& name) override;
    AbstractMenu* AddAction(const String& name, const String& actionId) override;
private:
    QtMainWindow* host_ = nullptr;
    QMenu* menu_ = nullptr;
    QAction* action_ = nullptr;
    QList<QtMenu> children_;
};

class QtMainWindow : public QMainWindow, public AbstractMainWindow
{
    Q_OBJECT

public:
    QtMainWindow(QApplication& application);
    ~QtMainWindow() override;

    AbstractDialog* AddDialog(DialogLocationHint hint) override;
    void AddAction(const AbstractAction& actionDesc) override;
    AbstractMenu* AddMenu(const String& name) override;

    Context* GetContext() override;
    AbstractInput* GetInput() override;

    QtUrhoWidget& GetUrhoWidget() { return urhoWidget_; }
    QAction* FindAction(const String& id) const;

private:
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateDummyWidget,      QtDummyWidget);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateScrollArea,       QtScrollArea);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLayout,           QtLayout);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateCollapsiblePanel, QtCollapsiblePanel);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateButton,           QtButton);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateText,             QtText);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateLineEdit,         QtLineEdit);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateCheckBox,         QtCheckBox);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateHierarchyList,    QtHierarchyList);
    URHO3D_IMPLEMENT_WIDGET_FACTORY(CreateView3D,           QtView3D);

private:
    SharedPtr<Context> context_;
    QApplication& application_;
    QtUrhoWidget urhoWidget_;
    Vector<SharedPtr<AbstractDialog>> dialogs_;

    HashMap<String, QAction*> actions_;
    QList<QtMenu> menus_;
};

}
