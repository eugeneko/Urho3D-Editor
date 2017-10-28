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
    String name_;
    Vector<SharedPtr<ResourceDirectoryDesc>> children_;
    Vector<SharedPtr<ResourceFileDesc>> files_;
    WeakPtr<ResourceDirectoryItem> item_;
};

/// Resource type recognition layer.
class ResourceRecognitionLayer : public RefCounted
{
public:
    virtual bool CanParseFileName() const { return false; }
    virtual ResourceType ParseFileName(const String& extension, const String& fullName);
    virtual bool CanParseFileID() const { return false; }
    virtual ResourceType ParseFileID(const String& fileId);
    virtual bool CanParseRootNode() const { return false; }
    virtual ResourceType ParseRootNode(const String& rootName);
};

/// Shared pointer onto resource type recognition layer.
using ResourceRecognitionLayerSPtr = SharedPtr<ResourceRecognitionLayer>;

/// Array of resource type recognition layers.
using ResourceRecognitionLayerArray = Vector<ResourceRecognitionLayerSPtr>;

class ExtensionResourceRecognitionLayer : public ResourceRecognitionLayer
{
public:
    ExtensionResourceRecognitionLayer(const String& extension, const String& resourceType, StringHash objectType = StringHash());

    virtual bool CanParseFileName() const { return true; }
    ResourceType ParseFileName(const String& extension, const String& fullName) override;

private:
    const String extension_;
    const ResourceType type_;
};

class BinaryResourceRecognitionLayer : public ResourceRecognitionLayer
{
public:
    BinaryResourceRecognitionLayer(const String& fileId, const String& resourceType, StringHash objectType = StringHash());

    bool CanParseFileID() const override { return true; }
    ResourceType ParseFileID(const String& fileId) override;

private:
    const String fileId_;
    const ResourceType type_;
};

class XMLResourceRecognitionLayer : public ResourceRecognitionLayer
{
public:
    XMLResourceRecognitionLayer(const String& rootName, const String& resourceType, StringHash objectType = StringHash());

    bool CanParseRootNode() const override { return true; }
    ResourceType ParseRootNode(const String& rootName) override;

private:
    const String rootName_;
    const ResourceType type_;
};

/// Create extension type recognition layer.
ResourceRecognitionLayerSPtr MakeExtensionLayer(const String& extension, const String& resourceType, StringHash objectType = StringHash());
/// Create extension type recognition layers.
ResourceRecognitionLayerArray MakeExtensionLayers(const Vector<String>& extensions, const String& resourceType, StringHash objectType = StringHash());
/// Create extension type recognition layer for object type.
template <class T> ResourceRecognitionLayerSPtr MakeExtensionLayer(const String& extension) { return MakeExtensionLayer(extension, T::GetTypeNameStatic(), T::GetTypeStatic()); }
/// Create extension type recognition layers for object type.
template <class T> ResourceRecognitionLayerArray MakeExtensionLayers(const Vector<String>& extensions) { return MakeExtensionLayers(extensions, T::GetTypeNameStatic(), T::GetTypeStatic()); }
/// Create binary type recognition layer.
ResourceRecognitionLayerSPtr MakeBinaryLayer(const String& fileId, const String& resourceType, StringHash objectType = StringHash());
/// Create binary type recognition layers.
ResourceRecognitionLayerArray MakeBinaryLayers(const Vector<String>& fileIds, const String& resourceType, StringHash objectType = StringHash());
/// Create binary type recognition layer for object type.
template <class T> ResourceRecognitionLayerSPtr MakeBinaryLayer(const String& fileId) { return MakeBinaryLayer(fileId, T::GetTypeNameStatic(), T::GetTypeStatic()); }
/// Create binary type recognition layer for object type.
template <class T> ResourceRecognitionLayerArray MakeBinaryLayers(const Vector<String>& fileIds) { return MakeBinaryLayers(fileIds, T::GetTypeNameStatic(), T::GetTypeStatic()); }
/// Create XML type recognition layer.
ResourceRecognitionLayerSPtr MakeXmlLayer(const String& rootName, const String& resourceType, StringHash objectType = StringHash());
/// Create XML type recognition layers.
ResourceRecognitionLayerArray MakeXmlLayers(const Vector<String>& rootNames, const String& resourceType, StringHash objectType = StringHash());
/// Create XML type recognition layer for object type.
template <class T> ResourceRecognitionLayerSPtr MakeXmlLayer(const String& rootName) { return MakeXmlLayer(rootName, T::GetTypeNameStatic(), T::GetTypeStatic()); }
/// Create XML type recognition layer for object type.
template <class T> ResourceRecognitionLayerArray MakeXmlLayers(const Vector<String>& rootNames) { return MakeXmlLayers(rootNames, T::GetTypeNameStatic(), T::GetTypeStatic()); }

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
    void AddLayer(const SharedPtr<ResourceRecognitionLayer>& layer);
    /// Add multiple resource type recognition layers.
    void AddLayers(const Vector<SharedPtr<ResourceRecognitionLayer>>& layers);
    /// Rescan resources.
    void ScanResources();

    /// Return selected directory.
    const ResourceDirectoryDesc* GetSelectedDirectory() const;
    /// Set selected directory.
    bool SelectDirectory(const String& directoryKey);

public:
    /// Called when file is clicked.
    std::function<void(const ResourceFileDesc& file)> onResourceClicked_;
    /// Called when file is double-clicked.
    std::function<void(const ResourceFileDesc& file)> onResourceDoubleClicked_;

private:
    /// Recursive sort of directory content.
    static void SortChildrenDirectories(ResourceDirectoryDesc& directory);
    /// Clean directory content.
    static void ClearDirectory(ResourceDirectoryDesc& directory);

    /// Get resource file type.
    ResourceType GetResourceType(const ResourceFileDesc& desc);

    /// Scan resource directory for directories and files.
    void ScanResourceDirectory(const String& path, unsigned resourceDirIndex);
    /// Add new directory with files.
    void AddDirectory(const String& directoryKey, const String& resourceDir, unsigned resourceDirIndex);
    /// Scan directory for files.
    void ScanResourceFiles(ResourceDirectoryDesc& directory, const String& resourceDir, unsigned resourceDirIndex);
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
