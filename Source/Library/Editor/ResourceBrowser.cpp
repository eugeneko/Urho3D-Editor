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
const ResourceType ResourceType::EMPTY;

ResourceType::ResourceType(const StringHash& objectType, const String& resourceType)
    : objectType_(objectType)
    , resourceType_(resourceType)
{
}

bool ResourceType::operator==(const ResourceType& rhs) const
{
    return objectType_ == rhs.objectType_
        && resourceType_ == rhs.resourceType_;
}

//////////////////////////////////////////////////////////////////////////
ResourceType ResourceRecognitionLayer::ParseFileName(const String& /*extension*/, const String& /*fullName*/)
{
    return ResourceType::EMPTY;
}

ResourceType ResourceRecognitionLayer::ParseFileID(const String& /*fileId*/)
{
    return ResourceType::EMPTY;
}

ResourceType ResourceRecognitionLayer::ParseRootNode(const String& /*rootName*/)
{
    return ResourceType::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
ExtensionResourceRecognitionLayer::ExtensionResourceRecognitionLayer(
    const String& extension, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : extension_(extension)
    , type_(objectType, resourceType)
{
}

ResourceType ExtensionResourceRecognitionLayer::ParseFileName(const String& extension, const String& /*fullName*/)
{
    if (extension_ == extension)
        return type_;
    return ResourceType::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
BinaryResourceRecognitionLayer::BinaryResourceRecognitionLayer(
    const String& fileId, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : fileId_(fileId)
    , type_(objectType, resourceType)
{
}

ResourceType BinaryResourceRecognitionLayer::ParseFileID(const String& fileId)
{
    if (fileId_ == fileId)
        return type_;
    return ResourceType::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
XMLResourceRecognitionLayer::XMLResourceRecognitionLayer(
    const String& rootName, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : rootName_(rootName)
    , type_(objectType, resourceType)
{
}

ResourceType XMLResourceRecognitionLayer::ParseRootNode(const String& rootName)
{
    if (rootName_ == rootName)
        return type_;
    return ResourceType::EMPTY;
}

//////////////////////////////////////////////////////////////////////////
ResourceRecognitionLayerSPtr MakeExtensionLayer(const String& extension, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    return MakeShared<ExtensionResourceRecognitionLayer>(extension, resourceType, objectType);
}

ResourceRecognitionLayerArray MakeExtensionLayers(const Vector<String>& extensions, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    ResourceRecognitionLayerArray result;
    for (const String& extension : extensions)
        result.Push(MakeExtensionLayer(extension, resourceType, objectType));
    return result;
}

ResourceRecognitionLayerSPtr MakeBinaryLayer(const String& fileId, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    return MakeShared<BinaryResourceRecognitionLayer>(fileId, resourceType, objectType);
}

ResourceRecognitionLayerArray MakeBinaryLayers(const Vector<String>& fileIds, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    ResourceRecognitionLayerArray result;
    for (const String& fileId : fileIds)
        result.Push(MakeBinaryLayer(fileId, resourceType, objectType));
    return result;
}

ResourceRecognitionLayerSPtr MakeXmlLayer(const String& rootName, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    return MakeShared<XMLResourceRecognitionLayer>(rootName, resourceType, objectType);

}

ResourceRecognitionLayerArray MakeXmlLayers(const Vector<String>& rootNames, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    ResourceRecognitionLayerArray result;
    for (const String& rootName : rootNames)
        result.Push(MakeXmlLayer(rootName, resourceType, objectType));
    return result;
}

//////////////////////////////////////////////////////////////////////////
ResourceBrowser::ResourceBrowser(AbstractMainWindow* mainWindow)
    : Object(mainWindow->GetContext())
    , fileSystem_(GetSubsystem<FileSystem>())
    , cache_(GetSubsystem<ResourceCache>())
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
    filesView_->onItemClicked_ = [=](AbstractHierarchyListItem* item)
    {
        if (onResourceClicked_)
            onResourceClicked_(*static_cast<ResourceFileItem*>(item)->GetDesc());
    };
    filesView_->onItemDoubleClicked_ = [=](AbstractHierarchyListItem* item)
    {
        if (onResourceDoubleClicked_)
            onResourceDoubleClicked_(*static_cast<ResourceFileItem*>(item)->GetDesc());
    };
}

void ResourceBrowser::AddXmlExtension(const String& extension)
{
    xmlExtensions_.Insert(extension);
}

void ResourceBrowser::AddLayer(const SharedPtr<ResourceRecognitionLayer>& layer)
{
    recognitionLayers_.Push(layer);
}

void ResourceBrowser::AddLayers(const Vector<SharedPtr<ResourceRecognitionLayer>>& layers)
{
    recognitionLayers_.Insert(recognitionLayers_.End(), layers);
}

void ResourceBrowser::ScanResources()
{
    // Remember selected directory
    String selectedDirectory;
    if (const ResourceDirectoryDesc* directory = GetSelectedDirectory())
        selectedDirectory = directory->directoryKey_;

    // Cleanup caches
    ClearDirectory(rootDirectory_);
    directories_.Clear();
    directories_[""] = &rootDirectory_;

    // Scan resources
    const Vector<String> resourceDirs = cache_->GetResourceDirs();
    for (unsigned i = 0; i < resourceDirs.Size(); ++i)
        ScanResourceDirectory(resourceDirs[i], i);

    // Sort
    SortChildrenDirectories(rootDirectory_);

    // Update UI
    directoriesView_->RemoveAllItems();
    filesView_->RemoveAllItems();
    UpdateDirectoryView(rootDirectory_, 0, nullptr);

    // Select directory
    SelectDirectory(selectedDirectory);

    // Update types
    for (auto& item : directories_)
    {
        ResourceDirectoryDesc& directory = *item.second_;
        for (ResourceFileDesc* file : directory.files_)
            file->type_ = GetResourceType(*file);
    }
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

ResourceType ResourceBrowser::GetResourceType(const ResourceFileDesc& desc)
{
    SharedPtr<File> file;
    String fileId;
    String rootNode;

    for (ResourceRecognitionLayer* layer : recognitionLayers_)
    {
        // Try to parse file name
        if (layer->CanParseFileName())
        {
            const ResourceType typeByName = layer->ParseFileName(desc.extension_, desc.resourceKey_);
            if (!typeByName.IsEmpty())
                return typeByName;
        }

        // Load file if needed
        if (layer->CanParseFileID() || layer->CanParseRootNode())
        {
            file = cache_->GetFile(desc.resourceKey_, false);
        }

        // Try to parse file ID
        if (layer->CanParseFileID())
        {
            // Load file ID
            if (fileId.Empty())
            {
                file->Seek(0);
                fileId = file->ReadFileID();
            }

            const ResourceType typeById = layer->ParseFileID(fileId);
            if (!typeById.IsEmpty())
                return typeById;
        }

        // Try to parse root node name
        if (layer->CanParseRootNode() && xmlExtensions_.Contains(desc.extension_))
        {
            // Load root node name
            if (rootNode.Empty())
            {
                file->Seek(0);
                XMLFile xml(context_);
                xml.Load(*file);
                rootNode = xml.GetRoot().GetName();
            }

            const ResourceType xmlType = layer->ParseRootNode(rootNode);
            if (!xmlType.IsEmpty())
                return xmlType;
        }
    }
    return ResourceType::EMPTY;
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

    ScanResourceFiles(rootDirectory_, path, resourceDirIndex);
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
            desc->name_ = parts[i];

            parent->children_.Push(desc);
            directories_[parentKey] = desc;
        }

        // Get the parent
        parent = directories_[parentKey];
    }

    ScanResourceFiles(*parent, resourceDir, resourceDirIndex);
}

void ResourceBrowser::ScanResourceFiles(ResourceDirectoryDesc& directory, const String& resourceDir, unsigned resourceDirIndex)
{
    Vector<String> files;
    fileSystem_->ScanDir(files, resourceDir + directory.directoryKey_, "*.*", SCAN_FILES, false);

    for (const String& file : files)
    {
        auto fileDesc = MakeShared<ResourceFileDesc>();
        fileDesc->name_ = file;
        fileDesc->extension_ = GetExtension(file);
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