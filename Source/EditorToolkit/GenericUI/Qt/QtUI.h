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

class QtDockDialog : public GenericDialog
{

public:
    QtDockDialog(AbstractMainWindow& mainWindow);
    void SetName(const String& name) override;


private:
    bool DoSetContent(GenericWidget* content) override;

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
    bool DoSetContent(GenericWidget* content) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

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
    bool DoSetCell(unsigned row, unsigned column, GenericWidget* child) override;
    bool DoSetRow(unsigned row, GenericWidget* child) override;
    void DoRemoveChild(GenericWidget* child) override;

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
    bool DoSetHeaderPrefix(GenericWidget* header) override;
    bool DoSetHeaderSuffix(GenericWidget* header) override;
    bool DoSetBody(GenericWidget* body) override;

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
    AbstractButton& SetText(const String& text) override;


private:
    QPushButton* pushButton_ = nullptr;

};

class QtText : public AbstractText
{
    URHO3D_OBJECT(AbstractText, GenericWidget);

public:
    QtText(AbstractMainWindow& mainWindow);
    AbstractText& SetText(const String& text) override;
    unsigned GetTextWidth() const override;

private:
    QLabel* label_ = nullptr;

};

class QtLineEdit : public QObject, public AbstractLineEdit
{
    Q_OBJECT
    URHO3D_OBJECT(AbstractLineEdit, GenericWidget);

public:
    QtLineEdit(AbstractMainWindow& mainWindow);
    AbstractLineEdit& SetText(const String& text) override;
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
    AbstractCheckBox& SetChecked(bool checked) override;

private:
    QCheckBox* checkBox_ = nullptr;
};

class QtHierarchyListModel : public QAbstractItemModel
{
public:
    QtHierarchyListModel(AbstractMainWindow& mainWindow);

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

class QtHierarchyList : public GenericHierarchyList
{
public:
    QtHierarchyList(AbstractMainWindow& mainWindow);

    void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) override;
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

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
