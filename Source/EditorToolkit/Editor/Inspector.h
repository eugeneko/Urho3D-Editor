#pragma once

#include "../GenericUI/GenericUI.h"

namespace Urho3D
{

class Inspectable : public Object
{
    URHO3D_OBJECT(Inspectable, Object);

public:
    Inspectable(Context* context) : Object(context) { }
    virtual void BuildUI(AbstractLayout* layout) = 0;

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
    std::function<void()> onEntered_;
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
    bool AddObject(Serializable* object);
    const PODVector<Serializable*>& GetObjects() const { return objects_; }
    unsigned GetNumObjects() const { return objects_.Size(); }

    void BuildUI(AbstractLayout* layout) override;

private:
    //AttributeEditor* BuildAttributeUI(AbstractLayout* layout, unsigned row);

private:
    unsigned maxLabelLength_ = M_MAX_UNSIGNED;
    PODVector<Serializable*> objects_;
    Vector<SharedPtr<AttributeEditor>> attributes_;

};

class InspectablePanel : public Object
{
    URHO3D_OBJECT(InspectablePanel, Object);

public:
    InspectablePanel(Context* context) : Object(context) { }

    virtual void BuildUI(AbstractCollapsiblePanel* panel) = 0;
};

class MultipleSerializableInspectorPanel : public InspectablePanel
{
    URHO3D_OBJECT(MultipleSerializableInspectorPanel, InspectablePanel);

public:
    MultipleSerializableInspectorPanel(Context* context) : InspectablePanel(context), content_(context) { }

    void SetMaxLabelLength(unsigned maxLength) { content_.SetMaxLabelLength(maxLength); }
    bool AddObject(Serializable* object) { return content_.AddObject(object); }

    void BuildUI(AbstractCollapsiblePanel* panel) override;

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
    void AddPanel(InspectablePanel* panel);

    void BuildUI(AbstractLayout* layout) override;

private:
    Vector<SharedPtr<InspectablePanel>> panels_;
};

class Inspector : public Object
{
    URHO3D_OBJECT(Inspector, Object);

public:
    Inspector(AbstractMainWindow& mainWindow);
    void SetInspectable(Inspectable* inspectable);

private:
    GenericDialog* dialog_ = nullptr;
    AbstractScrollArea* scrollRegion_ = nullptr;
    AbstractLayout* layout_ = nullptr;

    SharedPtr<Inspectable> inspectable_;
};

}
