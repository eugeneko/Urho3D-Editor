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
    virtual void BuildUI(AbstractLayout* layout, unsigned& row, unsigned column) = 0;
    virtual void SetValues(const Vector<Variant>& values) = 0;
    virtual void GetValues(Vector<Variant>& values) = 0;

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
    void BuildUI(AbstractLayout* layout, unsigned& row, unsigned column) override;
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

    bool AddObject(Serializable* object);

    void BuildUI(AbstractLayout* layout) override;

private:
    PODVector<Serializable*> objects_;
    Vector<SharedPtr<AttributeEditor>> attributes_;

};

class MultiplePanelInspector : public Inspectable
{
    URHO3D_OBJECT(MultiplePanelInspector, Inspectable);

public:
    MultiplePanelInspector(Context* context) : Inspectable(context) { }
    void AddPanel(Inspectable* panel);

    void BuildUI(AbstractLayout* layout) override;

private:
    Vector<SharedPtr<Inspectable>> panels_;
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
