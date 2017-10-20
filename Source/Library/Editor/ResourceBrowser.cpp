#include "ResourceBrowser.h"
#include <Urho3D/Container/Sort.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3D
{

ResourceDirectoryDesc::ResourceDirectoryDesc(const String& directoryKey, const String& name)
    : directoryKey_(directoryKey)
    , name_(name)
{
}

void ResourceDirectoryDesc::AddChild(const SharedPtr<ResourceDirectoryDesc>& child)
{
    children_.Push(child);
}

void ResourceDirectoryDesc::Clear()
{
    children_.Clear();
    //files_.Clear();
}

void ResourceDirectoryDesc::SortChildren()
{
    Sort(children_.Begin(), children_.End(),
        [](ResourceDirectoryDesc* lhs, ResourceDirectoryDesc* rhs)
    {
        return lhs->GetName() < rhs->GetName();
    });
    for (ResourceDirectoryDesc* child : children_)
        child->SortChildren();
}

//////////////////////////////////////////////////////////////////////////
ResourceDirectoryItem::ResourceDirectoryItem(Context* context, ResourceDirectoryDesc* directory)
    : AbstractHierarchyListItem(context)
    , directory_(directory)
{

}

//////////////////////////////////////////////////////////////////////////
ResourceBrowser::ResourceBrowser(AbstractMainWindow* mainWindow)
    : Object(mainWindow->GetContext())
    , fileSystem_(GetSubsystem<FileSystem>())
    , rootDirectory_("", "Root")
{
    dock_ = mainWindow->AddDock(DockLocation::Bottom);
    dock_->SetName("Resource Browser");

    layout_ = dock_->CreateContent<AbstractLayout>();
    directoriesView_ = layout_->CreateCell<AbstractHierarchyList>(0, 0);
}

void ResourceBrowser::ScanResources()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Scan resources
    rootDirectory_.Clear();
    const Vector<String> resourceDirs = cache->GetResourceDirs();
    for (unsigned i = 0; i < resourceDirs.Size(); ++i)
    {
        ScanResourceDirectory(resourceDirs[i], i);
    }

    // Sort
    rootDirectory_.SortChildren();

    // Update UI
    directoriesView_->RemoveAllItems();

    auto rootItem = new ResourceDirectoryItem(context_, &rootDirectory_);
    directoriesView_->AddItem(rootItem, 0, nullptr);
    UpdateDirectoryUI(rootItem);
}

void ResourceBrowser::ScanResourceDirectory(const String& path, unsigned resourceDirIndex)
{
    Vector<String> directories;
    fileSystem_->ScanDir(directories, path, "*.*", SCAN_DIRS, true);
    for (const String& directory : directories)
    {
        if (directory.EndsWith("."))
            continue;
        AddDirectory(directory);
    }
}

void ResourceBrowser::AddDirectory(const String& directoryKey)
{
    if (directories_.Contains(directoryKey))
        return;

    const Vector<String> parts = directoryKey.Split('/');

    // Iterate from root to trunk
    String parentKey;
    ResourceDirectoryDesc* parent = &rootDirectory_;
    for (unsigned i = 0; i < parts.Size(); ++i)
    {
        // Make path
        if (i > 0)
            parentKey += "/";
        parentKey += parts[i];

        // Create directory if missing
        if (!directories_.Contains(parentKey))
        {
            auto desc = MakeShared<ResourceDirectoryDesc>(parentKey, parts[i]);
            parent->AddChild(desc);
            directories_[parentKey] = desc;
        }

        // Get the parent
        parent = directories_[parentKey];
    }
}

void ResourceBrowser::UpdateDirectoryUI(ResourceDirectoryItem* directory)
{
    unsigned index = 0;
    for (ResourceDirectoryDesc* child : directory->GetDesc()->GetChildren())
    {
        auto childItem = new ResourceDirectoryItem(context_, child);
        directoriesView_->AddItem(childItem, index, directory);
        UpdateDirectoryUI(childItem);
        ++index;
    }
}

}