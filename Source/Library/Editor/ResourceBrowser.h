#pragma once

#include "../AbstractUI/AbstractUI.h"

namespace Urho3D
{

class FileSystem;

class ResourceFileDesc : public AbstractHierarchyListItem
{
    String fullName_;
    unsigned resourceDirIndex_ = 0;
    String name_;
};

class ResourceDirectoryDesc : public RefCounted
{
public:
    ResourceDirectoryDesc(const String& directoryKey, const String& name);

    void AddChild(const SharedPtr<ResourceDirectoryDesc>& child);
    void SortChildren();
    void Clear();

    const String& GetName() const { return name_; }

    const Vector<SharedPtr<ResourceDirectoryDesc>>& GetChildren() const { return children_; }

private:

    String directoryKey_;
    String name_;
    Vector<SharedPtr<ResourceDirectoryDesc>> children_;
    //Vector<ResourceFileDesc> files_;
};

class ResourceDirectoryItem : public AbstractHierarchyListItem
{
public:
    ResourceDirectoryItem(Context* context, ResourceDirectoryDesc* directory);
    ResourceDirectoryDesc* GetDesc() const { return directory_; }

    String GetText() override { return directory_->GetName(); }

private:

    ResourceDirectoryDesc* directory_ = nullptr;
};

class ResourceBrowser : public Object
{
    URHO3D_OBJECT(ResourceBrowser, Object);

public:
    ResourceBrowser(AbstractMainWindow* mainWindow);
    void ScanResources();

private:
    void ScanResourceDirectory(const String& path, unsigned resourceDirIndex);
    void AddDirectory(const String& directoryKey);
    void UpdateDirectoryUI(ResourceDirectoryItem* directory);

private:
    FileSystem* fileSystem_ = nullptr;

    AbstractDock* dock_ = nullptr;
    AbstractLayout* layout_ = nullptr;
    AbstractHierarchyList* directoriesView_ = nullptr;
    ResourceDirectoryDesc rootDirectory_;
    HashMap<String, ResourceDirectoryDesc*> directories_;
};

}
