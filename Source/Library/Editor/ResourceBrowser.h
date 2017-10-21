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

struct ResourceType
{
    static const ResourceType EMPTY;

    ResourceType() = default;
    ResourceType(const StringHash& objectType, const String& resourceType);
    bool IsEmpty() const { return *this == EMPTY; }
    bool operator == (const ResourceType& rhs) const;
    bool operator != (const ResourceType& rhs) const { return !(*this == rhs); }

    StringHash objectType_;
    String resourceType_;
};

class ResourceFileDesc : public RefCounted
{
public:
    String resourceKey_;
    unsigned resourceDirIndex_ = 0;
    String name_;
    String extension_;
    ResourceType type_;
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

class ResourceRecognitionLayer : public Object
{
    URHO3D_OBJECT(ResourceRecognitionLayer, Object);

public:
    ResourceRecognitionLayer(Context* context) : Object(context) { }
    virtual ResourceType ParseFileName(const String& extension, const String& fullName);
    virtual ResourceType ParseFileID(const String& fileId);
    virtual ResourceType ParseRootNode(const String& rootName);
};

class ExtensionResourceRecognitionLayer : public ResourceRecognitionLayer
{
    URHO3D_OBJECT(ExtensionResourceRecognitionLayer, ResourceRecognitionLayer);

public:
    ExtensionResourceRecognitionLayer(Context* context,
        const String& extension, const String& resourceType, StringHash objectType = StringHash());

    ResourceType ParseFileName(const String& extension, const String& fullName) override;

private:
    const String extension_;
    const ResourceType type_;
};

class BinaryResourceRecognitionLayer : public ResourceRecognitionLayer
{
public:
    BinaryResourceRecognitionLayer(Context* context,
        const String& fileId, const String& resourceType, StringHash objectType = StringHash());

    ResourceType ParseFileID(const String& fileId) override;

private:
    const String fileId_;
    const ResourceType type_;
};

class XMLResourceRecognitionLayer : public ResourceRecognitionLayer
{
public:
    XMLResourceRecognitionLayer(Context* context,
        const String& rootName, const String& resourceType, StringHash objectType = StringHash());

    ResourceType ParseRootNode(const String& rootName) override;

private:
    const String rootName_;
    const ResourceType type_;
};

/// Resource Browser dock.
class ResourceBrowser : public Object
{
    URHO3D_OBJECT(ResourceBrowser, Object);

public:
    /// Construct.
    ResourceBrowser(AbstractMainWindow* mainWindow);

    /// Add XML extension filter.
    void AddXmlExtension(const String& extension);
    /// Add resource type recognition layer.
    void AddTypeRecognitionLayer(const SharedPtr<ResourceRecognitionLayer>& layer);

    /// Add extension type recognition layer.
    void AddExtensionLayer(const String& extension, const String& resourceType, StringHash objectType = StringHash());
    /// Add extension type recognition layers.
    void AddExtensionLayers(const Vector<String>& extensions, const String& resourceType, StringHash objectType = StringHash());
    /// Add extension type recognition layer for object type.
    template <class T> void AddExtensionLayer(const String& extension) { AddExtensionLayer(extension, T::GetTypeNameStatic(), T::GetTypeStatic()); }
    /// Add extension type recognition layers for object type.
    template <class T> void AddExtensionLayers(const Vector<String>& extensions) { AddExtensionLayers(extensions, T::GetTypeNameStatic(), T::GetTypeStatic()); }
    /// Add binary type recognition layer.
    void AddBinaryLayer(const String& fileId, const String& resourceType, StringHash objectType = StringHash());
    /// Add binary type recognition layers.
    void AddBinaryLayers(const Vector<String>& fileIds, const String& resourceType, StringHash objectType = StringHash());
    /// Add binary type recognition layer for object type.
    template <class T> void AddBinaryLayer(const String& fileId) { AddBinaryLayer(fileId, T::GetTypeNameStatic(), T::GetTypeStatic()); }
    /// Add binary type recognition layer for object type.
    template <class T> void AddBinaryLayers(const Vector<String>& fileIds) { AddBinaryLayers(fileIds, T::GetTypeNameStatic(), T::GetTypeStatic()); }
    /// Add XML type recognition layer.
    void AddXmlLayer(const String& rootName, const String& resourceType, StringHash objectType = StringHash());
    /// Add XML type recognition layers.
    void AddXmlLayers(const Vector<String>& rootNames, const String& resourceType, StringHash objectType = StringHash());
    /// Add XML type recognition layer for object type.
    template <class T> void AddXmlLayer(const String& rootName) { AddXmlLayer(rootName, T::GetTypeNameStatic(), T::GetTypeStatic()); }
    /// Add XML type recognition layer for object type.
    template <class T> void AddXmlLayers(const Vector<String>& rootNames) { AddXmlLayers(rootNames, T::GetTypeNameStatic(), T::GetTypeStatic()); }

    /// Rescan resources.
    void ScanResources();

    /// Return selected directory.
    const ResourceDirectoryDesc* GetSelectedDirectory() const;
    /// Set selected directory.
    bool SelectDirectory(const String& directoryKey);

public:
    /// Called when file is clicked.
    /// Called when file is double-clicked.

private:
    /// Recursive sort of directory content.
    static void SortChildrenDirectories(ResourceDirectoryDesc& directory);
    /// Clean directory content.
    static void ClearDirectory(ResourceDirectoryDesc& directory);

    /// Get resource file type.
    ResourceType GetResourceType(const ResourceFileDesc& desc);
    /// Get resource file type by name.
    ResourceType GetResourceTypeByName(const String& extension, const String& fullName);
    /// Get resource file type by file ID.
    ResourceType GetResourceTypeByFileID(const String& fileID);
    /// Get resource file type by root node name.
    ResourceType GetResourceTypeByRootNode(const String& rootName);

    /// Scan resource directory for directories and files.
    void ScanResourceDirectory(const String& path, unsigned resourceDirIndex);
    /// Add new directory with files.
    void AddDirectory(const String& directoryKey, const String& resourceDir, unsigned resourceDirIndex);
    /// Scan directory for files.
    void ScanResourceFiles(ResourceDirectoryDesc& directory, unsigned resourceDirIndex);
    /// Update directory viewer UI.
    void UpdateDirectoryView(ResourceDirectoryDesc& directory, unsigned index, ResourceDirectoryItem* parent);
    /// Update file viewer UI.
    void UpdateFileView(ResourceDirectoryDesc& directory);

private:
    FileSystem* fileSystem_ = nullptr;
    ResourceCache* cache_ = nullptr;
    Vector<SharedPtr<ResourceRecognitionLayer>> recognitionLayers_;
    HashSet<String> xmlExtensions_;

    AbstractDock* dock_ = nullptr;
    AbstractLayout* layout_ = nullptr;
    AbstractHierarchyList* directoriesView_ = nullptr;
    AbstractHierarchyList* filesView_ = nullptr;

    ResourceDirectoryDesc rootDirectory_;
    HashMap<String, ResourceDirectoryDesc*> directories_;
};

}
