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
#include <QToolButton>

namespace Urho3D
{

class QtMainWindow;

class QtWidget
{
public:
    static QtWidget* FromInterface(GenericWidget* widget);
    QWidget* Initialize()
    {
        if (!widget_)
            widget_ = CreateWidget();
        return widget_;
    }
    QWidget* GetElement() { return widget_; }

private:
    virtual QWidget* CreateWidget() = 0;
    QWidget* widget_ = nullptr;
};

class QtDockDialog : public GenericDialog, public QtWidget
{

public:
    QtDockDialog(AbstractMainWindow& mainWindow, GenericWidget* parent) : GenericDialog(mainWindow, parent) { }
    void SetName(const String& name) override;


private:
    QWidget* CreateWidget() override;
    bool SetContent(GenericWidget* content) override;

private:
    QDockWidget* dock_ = nullptr;
};

class QtDummyWidget : public AbstractDummyWidget, public QtWidget
{
public:
    QtDummyWidget(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractDummyWidget(mainWindow, parent) { }

private:
    QWidget* CreateWidget() override;

};

class QtScrollArea : public QObject, public AbstractScrollArea, public QtWidget
{
public:
    QtScrollArea(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractScrollArea(mainWindow, parent) { }

    void SetDynamicWidth(bool dynamicWidth) override;



private:
    QWidget* CreateWidget() override;
    bool SetContent(GenericWidget* content) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    void UpdateContentSize();

private:
    bool dynamicWidth_ = false;
    QScrollArea* scrollArea_ = nullptr;

};

class QtLayout : public AbstractLayout, public QtWidget
{
public:
    QtLayout(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractLayout(mainWindow, parent) { }



private:
    QWidget* CreateWidget() override;
    bool SetCellWidget(unsigned row, unsigned column, GenericWidget* child) override;
    bool SetRowWidget(unsigned row, GenericWidget* child) override;
    void RemoveChild(GenericWidget* child) override;

private:
    QWidget* widget_ = nullptr;
    QGridLayout* layout_ = nullptr;

};

class QtCollapsiblePanel : public QObject, public AbstractCollapsiblePanel, public QtWidget
{
    Q_OBJECT

public:
    QtCollapsiblePanel(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractCollapsiblePanel(mainWindow, parent) { }

    void SetHeaderText(const String& text) override;
    void SetExpanded(bool expanded) override;


private:
    QWidget* CreateWidget() override;
    bool SetHeaderPrefix(GenericWidget* header) override;
    bool SetHeaderSuffix(GenericWidget* header) override;
    bool SetBody(GenericWidget* body) override;

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

class QtButton : public AbstractButton, public QtWidget
{
public:
    QtButton(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractButton(mainWindow, parent) { }
    AbstractButton& SetText(const String& text) override;


private:
    QWidget* CreateWidget() override;
    QPushButton* pushButton_ = nullptr;

};

class QtText : public AbstractText, public QtWidget
{
    URHO3D_OBJECT(AbstractText, GenericWidget);

public:
    QtText(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractText(mainWindow, parent) { }
    AbstractText& SetText(const String& text) override;

private:
    QWidget* CreateWidget() override;

private:
    QLabel* label_ = nullptr;

};

class QtLineEdit : public AbstractLineEdit, public QtWidget
{
    URHO3D_OBJECT(AbstractLineEdit, GenericWidget);

public:
    QtLineEdit(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractLineEdit(mainWindow, parent) { }
    AbstractLineEdit& SetText(const String& text) override;

private:
    QWidget* CreateWidget() override;

private:
    QLineEdit* lineEdit_ = nullptr;

};

class QtCheckBox : public AbstractCheckBox, public QtWidget
{
    URHO3D_OBJECT(AbstractCheckBox, QtCheckBox);

public:
    QtCheckBox(AbstractMainWindow& mainWindow, GenericWidget* parent) : AbstractCheckBox(mainWindow, parent) { }
    AbstractCheckBox& SetChecked(bool checked) override;

private:
    QWidget* CreateWidget() override;

private:
    QCheckBox* checkBox_ = nullptr;
};

class QtHierarchyListModel : public QAbstractItemModel
{
public:
    QtHierarchyListModel(AbstractMainWindow& mainWindow, GenericWidget* parent);

    void InsertItem(GenericHierarchyListItem* item, const QModelIndex& parentIndex);
    void RemoveItem(GenericHierarchyListItem* item, const QModelIndex& parentIndex, int hintRow = -1);

    QModelIndex GetIndex(GenericHierarchyListItem* item, QModelIndex hint = QModelIndex());
    GenericHierarchyListItem* GetItem(const QModelIndex& index) const;
private:

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }

    GenericHierarchyListItem rootItem_;

};

class QtHierarchyList : public GenericHierarchyList, public QtWidget
{
public:
    QtHierarchyList(AbstractMainWindow& mainWindow, GenericWidget* parent);

    void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) override;
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

private:
    QWidget* CreateWidget() override;

private:

    QTreeView* treeView_ = nullptr;
    QScopedPointer<QtHierarchyListModel> model_;

};

class QtMenu : public GenericMenu
{
public:
    QtMenu(QtMainWindow* host, QMenu* menu);
    QtMenu(QtMainWindow* host, QAction* action);
    GenericMenu* AddMenu(const String& name) override;
    GenericMenu* AddAction(const String& name, const String& actionId) override;
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

    GenericDialog* AddDialog(DialogLocationHint hint) override;
    void AddAction(const AbstractAction& actionDesc) override;
    GenericMenu* AddMenu(const String& name) override;

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

private:
    SharedPtr<Context> context_;
    QApplication& application_;
    QtUrhoWidget urhoWidget_;
    Vector<SharedPtr<GenericDialog>> dialogs_;

    HashMap<String, QAction*> actions_;
    QList<QtMenu> menus_;
};

}
