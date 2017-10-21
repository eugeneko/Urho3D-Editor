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
const Urho3D::ResourceType ResourceType::EMPTY;

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
ExtensionResourceRecognitionLayer::ExtensionResourceRecognitionLayer(Context* context,
    const String& extension, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : ResourceRecognitionLayer(context)
    , extension_(extension)
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
BinaryResourceRecognitionLayer::BinaryResourceRecognitionLayer(Context* context,
    const String& fileId, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : ResourceRecognitionLayer(context)
    , fileId_(fileId)
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
XMLResourceRecognitionLayer::XMLResourceRecognitionLayer(Context* context,
    const String& rootName, const String& resourceType, StringHash objectType /*= StringHash()*/)
    : ResourceRecognitionLayer(context)
    , rootName_(rootName)
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
}

void ResourceBrowser::AddXmlExtension(const String& extension)
{
    xmlExtensions_.Insert(extension);
}

void ResourceBrowser::AddTypeRecognitionLayer(const SharedPtr<ResourceRecognitionLayer>& layer)
{
    recognitionLayers_.Push(layer);
}

void ResourceBrowser::AddExtensionLayer(const String& extension, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    AddTypeRecognitionLayer(MakeShared<ExtensionResourceRecognitionLayer>(context_, extension, resourceType, objectType));
}

void ResourceBrowser::AddExtensionLayers(const Vector<String>& extensions, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    for (const String& extension : extensions)
        AddExtensionLayer(extension, resourceType, objectType);
}

void ResourceBrowser::AddBinaryLayer(const String& fileId, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    AddTypeRecognitionLayer(MakeShared<BinaryResourceRecognitionLayer>(context_, fileId, resourceType, objectType));
}

void ResourceBrowser::AddBinaryLayers(const Vector<String>& fileIds, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    for (const String& fileId : fileIds)
        AddBinaryLayer(fileId, resourceType, objectType);
}

void ResourceBrowser::AddXmlLayer(const String& rootName, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    AddTypeRecognitionLayer(MakeShared<XMLResourceRecognitionLayer>(context_, rootName, resourceType, objectType));
}

void ResourceBrowser::AddXmlLayers(const Vector<String>& rootNames, const String& resourceType, StringHash objectType /*= StringHash()*/)
{
    for (const String& rootName : rootNames)
        AddXmlLayer(rootName, resourceType, objectType);
}

void ResourceBrowser::ScanResources()
{
    // Remember selected directory
    String selectedDirectory;
    if (const ResourceDirectoryDesc* directory = GetSelectedDirectory())
        selectedDirectory = directory->directoryKey_;

    // Scan resources
    ClearDirectory(rootDirectory_);
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
    if (!selectedDirectory.Empty())
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
    const ResourceType typeByName = GetResourceTypeByName(desc.extension_, desc.resourceKey_);
    if (!typeByName.IsEmpty())
        return typeByName;

    SharedPtr<File> file = cache_->GetFile(desc.resourceKey_, false);
    const String fileId = file->ReadFileID();
    const ResourceType typeById = GetResourceTypeByFileID(fileId);
    if (!typeById.IsEmpty())
        return typeById;

    if (xmlExtensions_.Contains(desc.extension_))
    {
        file->Seek(0);
        XMLFile xml(context_);
        xml.Load(*file);
        const ResourceType xmlType = GetResourceTypeByRootNode(xml.GetRoot().GetName());
        if (!xmlType.IsEmpty())
            return xmlType;
    }
    return ResourceType::EMPTY;
}

ResourceType ResourceBrowser::GetResourceTypeByName(const String& extension, const String& fullName)
{
    for (ResourceRecognitionLayer* layer : recognitionLayers_)
    {
        const ResourceType type = layer->ParseFileName(extension, fullName);
        if (!type.IsEmpty())
            return type;
    }
    return ResourceType::EMPTY;
}

ResourceType ResourceBrowser::GetResourceTypeByFileID(const String& fileID)
{
    for (ResourceRecognitionLayer* layer : recognitionLayers_)
    {
        const ResourceType type = layer->ParseFileID(fileID);
        if (!type.IsEmpty())
            return type;
    }
    return ResourceType::EMPTY;
}

ResourceType ResourceBrowser::GetResourceTypeByRootNode(const String& rootName)
{
    for (ResourceRecognitionLayer* layer : recognitionLayers_)
    {
        const ResourceType type = layer->ParseRootNode(rootName);
        if (!type.IsEmpty())
            return type;
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