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

namespace Urho3D
{

class QtMainWindow;

class QtDockDialog : public GenericDialog, public QDockWidget
{

public:
    QtDockDialog(AbstractUI& ui, GenericWidget* parent) : GenericDialog(ui, parent), QDockWidget() { }
    void SetBodyWidget(GenericWidget* widget) override;
    void SetName(const String& name) override;

};

class QtHierarchyListModel : public QAbstractItemModel
{
public:
    QtHierarchyListModel(AbstractUI& ui, GenericWidget* parent);

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

class QtHierarchyList : public GenericHierarchyList, public QTreeView
{
public:
    QtHierarchyList(AbstractUI& ui, GenericWidget* parent);

    void AddItem(GenericHierarchyListItem* item, unsigned index, GenericHierarchyListItem* parent) override;
    void SelectItem(GenericHierarchyListItem* item) override;
    void DeselectItem(GenericHierarchyListItem* item) override;
    void GetSelection(ItemVector& result) override;

private:

    QtHierarchyListModel model_;

};

class QtUI : public AbstractUI
{
public:
    QtUI(QtMainWindow& mainWindow) : mainWindow_(mainWindow) { }
    GenericWidget* CreateWidget(StringHash type, GenericWidget* parent) override;
    Context* GetContext() override;
    GenericMainWindow* GetMainWindow() override;
    AbstractInput* GetInput() override;

private:
    QtMainWindow& mainWindow_;

};

class QtMainWindow : public GenericMainWindow
{
public:
    QtMainWindow(QApplication& application);
    Context* GetContext() { return context_; }
    GenericDialog* AddDialog(DialogLocationHint hint) override;
    QtUrhoWidget& GetUrhoWidget() { return urhoWidget_; }
private:
    SharedPtr<Context> context_;
    QApplication& application_;
    QMainWindow mainWindow_;
    QtUrhoWidget urhoWidget_;
    QtUI ui_;
};

}
