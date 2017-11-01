#pragma once

#include "../AbstractUI/AbstractUI.h"

namespace Urho3D
{

namespace AttributeMetadata
{
    static const StringHash P_APPLY_ON_COMMIT("ApplyOnCommit");
}

class AttributeMetadataInjector : public Object
{
    URHO3D_OBJECT(AttributeMetadataInjector, Object);

public:
    AttributeMetadataInjector(Context* context) : Object(context) { }

    void AddMetadata(StringHash objectType, const String& attributeName, StringHash metadataKey, const Variant& metadata)
    {
        injectedMetadata_[objectType][attributeName][metadataKey] = metadata;
    }

    const Variant& GetMetadata(StringHash objectType, const String& attributeName, StringHash metadataKey)
    {
        auto objectMetadata = injectedMetadata_.Find(objectType);
        if (objectMetadata == injectedMetadata_.End())
            return Variant::EMPTY;

        auto attributeMetadata = objectMetadata->second_.Find(attributeName);
        if (attributeMetadata == objectMetadata->second_.End())
            return Variant::EMPTY;

        auto metadata = attributeMetadata->second_.Find(metadataKey);
        if (metadata == attributeMetadata->second_.End())
            return Variant::EMPTY;

        return metadata->second_;
    }

private:
    HashMap<StringHash, HashMap<String, HashMap<StringHash, Variant>>> injectedMetadata_;
};

class Inspectable : public Object
{
    URHO3D_OBJECT(Inspectable, Object);

public:
    Inspectable(Context* context) : Object(context) { }
    virtual void BuildUI(AbstractLayout* layout) = 0;
    virtual void Refresh() = 0;

};

class AttributeEditor : public Object
{
    URHO3D_OBJECT(AttributeEditor, Object);

public:
    AttributeEditor(Context* context) : Object(context) { }
    virtual void BuildUI(AbstractLayout* layout, unsigned row, bool occupyRow) = 0;
    virtual void SetValues(const Vector<Variant>& values) = 0;
    virtual void GetValues(Vector<Variant>& values) = 0;

public:
    std::function<void()> onChanged_;
    std::function<void()> onCommitted_;
};

class StringAttributeEditor : public AttributeEditor
{
    URHO3D_OBJECT(StringAttributeEditor, AttributeEditor);

public:
    StringAttributeEditor(Context* context) : AttributeEditor(context) { }
};

class VectorAttributeEditor : public AttributeEditor
{
    URHO3D_OBJECT(VectorAttributeEditor, AttributeEditor);

public:
    VectorAttributeEditor(Context* context, unsigned numComponents);
    void BuildUI(AbstractLayout* layout, unsigned row, bool occupyRow) override;
    void SetValues(const Vector<Variant>& values) override;
    void GetValues(Vector<Variant>& values) override;

private:
    void UnpackVariant(const Variant& source, float dest[]) const;
    void PackVariant(Variant& dest, float source[]) const;

private:
    unsigned numComponents_ = 1;
    AbstractLayout* internalLayout_ = nullptr;
    PODVector<AbstractLineEdit*> componentEditors_;
    bool componentsDefined_[4];
    float componentsValues_[4];
};

class MultipleSerializableInspector : public Inspectable
{
    URHO3D_OBJECT(MultipleSerializableInspector, Inspectable);

public:
    MultipleSerializableInspector(Context* context) : Inspectable(context) { }

    void SetMaxLabelLength(unsigned maxLength) { maxLabelLength_ = maxLength; }
    void SetMetadataInjector(const SharedPtr<AttributeMetadataInjector>& metadataInjector) { metadataInjector_ = metadataInjector; }

    bool AddObject(Serializable* object);
    const PODVector<Serializable*>& GetObjects() const { return objects_; }
    unsigned GetNumObjects() const { return objects_.Size(); }

    void BuildUI(AbstractLayout* layout) override;
    void Refresh() override;

private:
    SharedPtr<AttributeEditor> CreateAttributeEditor(unsigned attributeIndex, const AttributeInfo& attributeInfo);
    const Variant& GetAttributeMetadata(StringHash objectType, const AttributeInfo& attributeInfo, StringHash metadataKey);
    void LoadAttributeValues(unsigned attributeIndex, Vector<Variant>& values);
    void StoreAttributeValues(unsigned attributeIndex, const Vector<Variant>& values);
    void HandleAttributeChanged(unsigned attributeIndex);
    void HandleAttribureCommitted(unsigned attributeIndex);

private:
    unsigned maxLabelLength_ = M_MAX_UNSIGNED;
    SharedPtr<AttributeMetadataInjector> metadataInjector_;

    PODVector<Serializable*> objects_;
    StringHash objectType_;
    Vector<SharedPtr<AttributeEditor>> attributeEditors_;

    Vector<Variant> attributeValues_;
};

class InspectablePanel : public Object
{
    URHO3D_OBJECT(InspectablePanel, Object);

public:
    InspectablePanel(Context* context) : Object(context) { }

    virtual void BuildUI(AbstractCollapsiblePanel* panel) = 0;
    virtual void Refresh() = 0;
};

class MultipleSerializableInspectorPanel : public InspectablePanel
{
    URHO3D_OBJECT(MultipleSerializableInspectorPanel, InspectablePanel);

public:
    MultipleSerializableInspectorPanel(Context* context) : InspectablePanel(context), content_(context) { }

    void SetMaxLabelLength(unsigned maxLength);
    void SetMetadataInjector(const SharedPtr<AttributeMetadataInjector>& metadataInjector);

    bool AddObject(Serializable* object);

    void BuildUI(AbstractCollapsiblePanel* panel) override;
    void Refresh() override;

private:
    MultipleSerializableInspector content_;

    AbstractCheckBox* enabledCheckBox_ = nullptr;
    AbstractLayout* contentLayout_ = nullptr;
};

class MultiplePanelInspectable : public Inspectable
{
    URHO3D_OBJECT(MultiplePanelInspectable, Inspectable);

public:
    MultiplePanelInspectable(Context* context) : Inspectable(context) { }
    void AddPanel(const SharedPtr<InspectablePanel>& panel);

    void BuildUI(AbstractLayout* layout) override;
    void Refresh() override;

private:
    Vector<SharedPtr<InspectablePanel>> panels_;
};

class Inspector : public Object
{
    URHO3D_OBJECT(Inspector, Object);

public:
    Inspector(AbstractMainWindow* mainWindow);
    void SetInspectable(const SharedPtr<Inspectable>& inspectable);
    void Refresh();

private:
    AbstractDock* dialog_ = nullptr;
    AbstractScrollArea* scrollRegion_ = nullptr;
    AbstractLayout* layout_ = nullptr;

    SharedPtr<Inspectable> inspectable_;
};

}
