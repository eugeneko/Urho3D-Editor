#include "ResourceBrowser.h"
#include <Urho3D/Container/Sort.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Resource/ResourceCache.h>

namespace Urho3D
{

ResourceFileItem::ResourceFileItem(Context* context, ResourceFileDesc* file)
    : AbstractHierarchyListItem(context)
    , file_(file)
{
    file->item_ = this;
}

String ResourceFileItem::GetText()
{
    return file_->name_;
}

//////////////////////////////////////////////////////////////////////////
ResourceDirectoryItem::ResourceDirectoryItem(Context* context, ResourceDirectoryDesc* directory)
    : AbstractHierarchyListItem(context)
    , directory_(directory)
{
    directory_->item_ = this;
}

String ResourceDirectoryItem::GetText()
{
    return directory_->name_;
}

//////////////////////////////////////////////////////////////////////////
ResourceBrowser::ResourceBrowser(AbstractMainWindow* mainWindow)
    : Object(mainWindow->GetContext())
    , fileSystem_(GetSubsystem<FileSystem>())
{
    rootDirectory_.name_ = "Root";

    dock_ = mainWindow->AddDock(DockLocation::Bottom);
    dock_->SetName("Resource Browser");

    layout_ = dock_->CreateContent<AbstractLayout>();
    directoriesView_ = layout_->CreateCell<AbstractHierarchyList>(0, 0);
    directoriesView_->onItemClicked_ = [=](AbstractHierarchyListItem* item)
    {
        UpdateFileView(*static_cast<ResourceDirectoryItem*>(item)->GetDesc());
    };
    filesView_ = layout_->CreateCell<AbstractHierarchyList>(0, 1);
}

void ResourceBrowser::ScanResources()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Remember selected directory
    String selectedDirectory;
    if (const ResourceDirectoryDesc* directory = GetSelectedDirectory())
        selectedDirectory = directory->directoryKey_;

    // Scan resources
    ClearDirectory(rootDirectory_);
    const Vector<String> resourceDirs = cache->GetResourceDirs();
    for (unsigned i = 0; i < resourceDirs.Size(); ++i)
        ScanResourceDirectory(resourceDirs[i], i);

    // Sort
    SortChildrenDirectories(rootDirectory_);

    // Update UI
    directoriesView_->RemoveAllItems();
    filesView_->RemoveAllItems();
    UpdateDirectoryView(rootDirectory_, 0, nullptr);

    // Select directory
    if (!selectedDirectory.Empty())
        SelectDirectory(selectedDirectory);
}

const ResourceDirectoryDesc* ResourceBrowser::GetSelectedDirectory() const
{
    const AbstractHierarchyList::ItemVector selection = directoriesView_->GetSelection();
    if (selection.Size() == 1)
        return static_cast<ResourceDirectoryItem*>(selection[0])->GetDesc();
    return nullptr;
}

bool ResourceBrowser::SelectDirectory(const String& directoryKey)
{
    if (!directories_.Contains(directoryKey))
        return false;

    ResourceDirectoryDesc* directory = directories_[directoryKey];
    if (ResourceDirectoryItem* item = directory->item_)
    {
        directoriesView_->SelectItem(item);
        UpdateFileView(*directory);
    }

    return true;
}

void ResourceBrowser::SortChildrenDirectories(ResourceDirectoryDesc& directory)
{
    // Sort directories
    Sort(directory.children_.Begin(), directory.children_.End(),
        [](ResourceDirectoryDesc* lhs, ResourceDirectoryDesc* rhs)
    {
        return lhs->name_ < rhs->name_;
    });

    // Sort files
    Sort(directory.files_.Begin(), directory.files_.End(),
        [](ResourceFileDesc* lhs, ResourceFileDesc* rhs)
    {
        return lhs->name_ < rhs->name_;
    });

    // Recurse
    for (ResourceDirectoryDesc* child : directory.children_)
        SortChildrenDirectories(*child);
}

void ResourceBrowser::ClearDirectory(ResourceDirectoryDesc& directory)
{
    directory.children_.Clear();
    directory.files_.Clear();
}

void ResourceBrowser::ScanResourceDirectory(const String& path, unsigned resourceDirIndex)
{
    Vector<String> directories;
    fileSystem_->ScanDir(directories, path, "*.*", SCAN_DIRS, true);
    for (const String& directory : directories)
    {
        if (directory.EndsWith("."))
            continue;
        AddDirectory(directory, path, resourceDirIndex);
    }
}

void ResourceBrowser::AddDirectory(const String& directoryKey, const String& resourceDir, unsigned resourceDirIndex)
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
            auto desc = MakeShared<ResourceDirectoryDesc>();
            desc->directoryKey_ = parentKey;
            desc->fullPath_ = resourceDir + parentKey;
            desc->name_ = parts[i];

            parent->children_.Push(desc);
            directories_[parentKey] = desc;
        }

        // Get the parent
        parent = directories_[parentKey];
    }

    ScanResourceFiles(*parent, resourceDirIndex);
}

void ResourceBrowser::ScanResourceFiles(ResourceDirectoryDesc& directory, unsigned resourceDirIndex)
{
    Vector<String> files;
    fileSystem_->ScanDir(files, directory.fullPath_, "*.*", SCAN_FILES, false);

    for (const String& file : files)
    {
        auto fileDesc = MakeShared<ResourceFileDesc>();
        fileDesc->name_ = file;
        fileDesc->resourceKey_ = directory.directoryKey_ + "/" + file;
        fileDesc->resourceDirIndex_ = resourceDirIndex;
        directory.files_.Push(fileDesc);
    }
}

void ResourceBrowser::UpdateDirectoryView(ResourceDirectoryDesc& directory, unsigned index, ResourceDirectoryItem* parent)
{
    auto directoryItem = MakeShared<ResourceDirectoryItem>(context_, &directory);
    directoriesView_->AddItem(directoryItem, index, parent);
    for (unsigned i = 0; i < directory.children_.Size(); ++i)
        UpdateDirectoryView(*directory.children_[i], i, directoryItem);
}

void ResourceBrowser::UpdateFileView(ResourceDirectoryDesc& directory)
{
    filesView_->RemoveAllItems();
    for (unsigned i = 0; i < directory.files_.Size(); ++i)
    {
        auto fileItem = MakeShared<ResourceFileItem>(context_, directory.files_[i]);
        filesView_->AddItem(fileItem, i, nullptr);
    }
}

}