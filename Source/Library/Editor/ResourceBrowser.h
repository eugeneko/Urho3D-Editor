#pragma once

#include "../AbstractUI/AbstractUI.h"

namespace Urho3D
{

class FileSystem;

class ResourceFileDesc;
class ResourceDirectoryDesc;

class ResourceFileItem : public AbstractHierarchyListItem
{
public:
    ResourceFileItem(Context* context, ResourceFileDesc* directory);
    ResourceFileDesc* GetDesc() const { return file_; }

    String GetText() override;

private:

    ResourceFileDesc* file_ = nullptr;
};

class ResourceDirectoryItem : public AbstractHierarchyListItem
{
public:
    ResourceDirectoryItem(Context* context, ResourceDirectoryDesc* directory);
    ResourceDirectoryDesc* GetDesc() const { return directory_; }

    String GetText() override;

private:

    ResourceDirectoryDesc* directory_ = nullptr;
};

class ResourceFileDesc : public RefCounted
{
public:
    String resourceKey_;
    unsigned resourceDirIndex_ = 0;
    String name_;
    WeakPtr<ResourceFileItem> item_;
};

class ResourceDirectoryDesc : public RefCounted
{
public:
    String directoryKey_;
    String fullPath_;
    String name_;
    Vector<SharedPtr<ResourceDirectoryDesc>> children_;
    Vector<SharedPtr<ResourceFileDesc>> files_;
    WeakPtr<ResourceDirectoryItem> item_;
};

class ResourceBrowser : public Object
{
    URHO3D_OBJECT(ResourceBrowser, Object);

public:
    ResourceBrowser(AbstractMainWindow* mainWindow);
    void ScanResources();
    const ResourceDirectoryDesc* GetSelectedDirectory() const;
    bool SelectDirectory(const String& directoryKey);

private:
    static void SortChildrenDirectories(ResourceDirectoryDesc& directory);
    static void ClearDirectory(ResourceDirectoryDesc& directory);

    void ScanResourceDirectory(const String& path, unsigned resourceDirIndex);
    void AddDirectory(const String& directoryKey, const String& resourceDir, unsigned resourceDirIndex);
    void ScanResourceFiles(ResourceDirectoryDesc& directory, unsigned resourceDirIndex);
    void UpdateDirectoryView(ResourceDirectoryDesc& directory, unsigned index, ResourceDirectoryItem* parent);
    void UpdateFileView(ResourceDirectoryDesc& directory);

private:
    FileSystem* fileSystem_ = nullptr;

    AbstractDock* dock_ = nullptr;
    AbstractLayout* layout_ = nullptr;
    AbstractHierarchyList* directoriesView_ = nullptr;
    AbstractHierarchyList* filesView_ = nullptr;
    ResourceDirectoryDesc rootDirectory_;
    HashMap<String, ResourceDirectoryDesc*> directories_;
};

}
