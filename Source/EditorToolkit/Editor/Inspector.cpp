#include "Inspector.h"
#include <Urho3D/Core/StringUtils.h>

namespace Urho3D
{

VectorAttributeEditor::VectorAttributeEditor(Context* context, unsigned numComponents)
    : AttributeEditor(context)
    , numComponents_(numComponents)
{
    assert(numComponents >= 1 && numComponents <= 4);
}

void VectorAttributeEditor::BuildUI(AbstractLayout* layout, unsigned row, bool occupyRow)
{
    if (occupyRow)
        internalLayout_ = layout->CreateRow<AbstractLayout>(row);
    else
        internalLayout_ = layout->CreateCell<AbstractLayout>(row, 1);

    unsigned cell = 0;
    const char* labels[] = { "X", "Y", "Z", "W" };
    for (unsigned i = 0; i < numComponents_; ++i)
    {
        AbstractText* componentLabel = internalLayout_->CreateCell<AbstractText>(0, cell++);
        componentLabel->SetText(labels[i]);
        AbstractLineEdit* componentValue = internalLayout_->CreateCell<AbstractLineEdit>(0, cell++);
        componentValue->onTextEdited_ = [=]()
        {
            componentsDefined_[i] = true;
            componentsValues_[i] = ToFloat(componentValue->GetText());
            if (onChanged_)
                onChanged_();
        };
        componentValue->onTextFinished_ = [=]()
        {
            if (onCommitted_)
                onCommitted_();
        };
        componentEditors_.Push(componentValue);
    }
}

void VectorAttributeEditor::SetValues(const Vector<Variant>& values)
{
    for (unsigned i = 0; i < numComponents_; ++i)
        componentsDefined_[i] = true;
    UnpackVariant(values[0], componentsValues_);

    for (unsigned i = 1; i < values.Size(); ++i)
    {
        float components[4];
        UnpackVariant(values[i], components);
        for (unsigned j = 0; j < numComponents_; ++j)
        {
            if (!Equals(componentsValues_[j], components[j]))
                componentsDefined_[j] = false;
        }
    }

    for (unsigned i = 0; i < numComponents_; ++i)
    {
        if (componentsDefined_[i])
            componentEditors_[i]->SetText(String(componentsValues_[i]));
        else
            componentEditors_[i]->SetText("--");
    }
}

void VectorAttributeEditor::GetValues(Vector<Variant>& values)
{
    for (unsigned i = 0; i < values.Size(); ++i)
    {
        float components[4];
        UnpackVariant(values[i], components);
        for (unsigned j = 0; j < numComponents_; ++j)
            if (componentsDefined_[j])
                components[j] = componentsValues_[j];
        PackVariant(values[i], components);
    }

}

void VectorAttributeEditor::UnpackVariant(const Variant& source, float dest[]) const
{
    switch (numComponents_)
    {
    case 1:
        dest[0] = source.GetFloat();
        break;

    case 2:
        {
            const Vector2 vec = source.GetVector2();
            dest[0] = vec.x_;
            dest[1] = vec.y_;
        }
        break;

    case 3:
        {
            const Vector3 vec = source.GetVector3();
            dest[0] = vec.x_;
            dest[1] = vec.y_;
            dest[2] = vec.z_;
        }
        break;

    case 4:
        {
            const Vector4 vec = source.GetVector4();
            dest[0] = vec.x_;
            dest[1] = vec.y_;
            dest[2] = vec.z_;
            dest[3] = vec.w_;
        }
        break;

    default:
        assert(0);
    }
}

void VectorAttributeEditor::PackVariant(Variant& dest, float source[]) const
{
    switch (numComponents_)
    {
    case 1:
        dest = source[0];
        break;

    case 2:
        dest = Vector2(source[0], source[1]);
        break;

    case 3:
        dest = Vector3(source[0], source[1], source[2]);
        break;

    case 4:
        dest = Vector4(source[0], source[1], source[2], source[4]);
        break;

    default:
        assert(0);
    }
}

//////////////////////////////////////////////////////////////////////////
bool MultipleSerializableInspector::AddObject(Serializable* object)
{
    if (!objects_.Empty())
    {
        // All objects must have the same type
        if (objects_[0]->GetType() != object->GetType())
            return false;
        // All object must have the same number of attributes
        if (objects_[0]->GetNumAttributes() != object->GetNumAttributes())
            return false;
    }

    objectType_ = object->GetType();
    objects_.Push(object);
    return true;
}

void MultipleSerializableInspector::BuildUI(AbstractLayout* layout)
{
    // Skip if nothing to render
    if (objects_.Empty() || !objects_[0]->GetAttributes())
        return;

    const Vector<AttributeInfo>& attributes = *objects_[0]->GetAttributes();
    Vector<Variant> values;
    unsigned row = 0;
    for (unsigned i = 0; i < attributes.Size(); ++i)
    {
        const AttributeInfo& attributeInfo = attributes[i];

        // Create text at row and check the width
        AbstractText* attributeNameText = layout->CreateRow<AbstractText>(row);
        attributeNameText->SetText(attributeInfo.name_);

        // Re-create text if small enough
        const bool occupyRow = attributeNameText->GetTextWidth() > maxLabelLength_;
        if (occupyRow)
            ++row;
        else
        {
            layout->RemoveRow(row);
            attributeNameText = layout->CreateCell<AbstractText>(row, 0);
            attributeNameText->SetText(attributeInfo.name_);
        }

        // Try to create attribute editor
        SharedPtr<AttributeEditor> attributeEditor = CreateAttributeEditor(i, attributeInfo);
        attributeEditors_.Push(attributeEditor);
        if (attributeEditor)
        {
            const bool applyOnCommit = GetAttributeMetadata(objectType_, attributeInfo, AttributeMetadata::P_APPLY_ON_COMMIT).GetBool();

            attributeEditor->BuildUI(layout, row, occupyRow);

            values.Clear();
            for (Serializable* object : objects_)
                values.Push(object->GetAttribute(i));

            attributeEditor->SetValues(values);
            attributeEditor->onChanged_ = [=]()
            {
                if (!applyOnCommit)
                    HandleAttributeChanged(i);
            };
            attributeEditor->onCommitted_ = [=]()
            {
                if (applyOnCommit)
                    HandleAttributeChanged(i);
                HandleAttribureCommitted(i);
            };
        }

        ++row;
    }
    layout->CreateRow<AbstractDummyWidget>(row);
}

SharedPtr<AttributeEditor> MultipleSerializableInspector::CreateAttributeEditor(
    unsigned attributeIndex, const AttributeInfo& attributeInfo)
{
    switch (attributeInfo.type_)
    {
    case VAR_VECTOR3:
        {
            auto editor = MakeShared<VectorAttributeEditor>(context_, 3);
            return editor;
        }
    default:
        return nullptr;
    }
}

const Variant& MultipleSerializableInspector::GetAttributeMetadata(
    StringHash objectType, const AttributeInfo& attributeInfo, StringHash metadataKey)
{
    if (metadataInjector_)
    {
        const Variant& injectedMetadata = metadataInjector_->GetMetadata(objectType, attributeInfo.name_, metadataKey);
        if (!injectedMetadata.IsEmpty())
            return injectedMetadata;
    }
    return attributeInfo.GetMetadata(metadataKey);

}

void MultipleSerializableInspector::LoadAttributeValues(unsigned attributeIndex, Vector<Variant>& values)
{
    values.Resize(objects_.Size());
    for (unsigned i = 0; i < objects_.Size(); ++i)
        values[i] = objects_[i]->GetAttribute(attributeIndex);
}

void MultipleSerializableInspector::StoreAttributeValues(unsigned attributeIndex, const Vector<Variant>& values)
{
    assert(values.Size() == objects_.Size());
    for (unsigned i = 0; i < objects_.Size(); ++i)
        objects_[i]->SetAttribute(attributeIndex, values[i]);
}

void MultipleSerializableInspector::HandleAttributeChanged(unsigned attributeIndex)
{
    AttributeEditor* attributeEditor = attributeEditors_[attributeIndex];
    if (!attributeEditor)
        return;

    // Update serializable attribute values
    LoadAttributeValues(attributeIndex, attributeValues_);
    attributeEditor->GetValues(attributeValues_);
    StoreAttributeValues(attributeIndex, attributeValues_);

    // Update values in UI
    LoadAttributeValues(attributeIndex, attributeValues_);
    attributeEditor->SetValues(attributeValues_);
}

void MultipleSerializableInspector::HandleAttribureCommitted(unsigned attributeIndex)
{
    if (!attributeEditors_[attributeIndex])
        return;
}

//////////////////////////////////////////////////////////////////////////
void MultipleSerializableInspectorPanel::BuildUI(AbstractCollapsiblePanel* panel)
{
    if (content_.GetNumObjects() == 0)
        return;

    const String typeName = content_.GetObjects()[0]->GetTypeName();
    enabledCheckBox_ = panel->CreateHeaderPrefix<AbstractCheckBox>();
    panel->SetHeaderText(typeName);
    contentLayout_ = panel->CreateBody<AbstractLayout>();
    content_.BuildUI(contentLayout_);
}

//////////////////////////////////////////////////////////////////////////
void MultiplePanelInspectable::AddPanel(InspectablePanel* panel)
{
    panels_.Push(SharedPtr<InspectablePanel>(panel));
}

void MultiplePanelInspectable::BuildUI(AbstractLayout* layout)
{
    unsigned row = 0;
    for (InspectablePanel* panel : panels_)
    {
        AbstractCollapsiblePanel* collapsiblePanel = layout->CreateRow<AbstractCollapsiblePanel>(row);
        collapsiblePanel->SetExpanded(true);
        panel->BuildUI(collapsiblePanel);
        ++row;
    }
}

//////////////////////////////////////////////////////////////////////////
Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockRight);
    dialog_->SetName("Inspector");
    scrollRegion_ = dialog_->CreateContent<AbstractScrollArea>();
    layout_ = scrollRegion_->CreateContent<AbstractLayout>();
}

void Inspector::SetInspectable(Inspectable* inspectable)
{
    inspectable_ = inspectable;
    layout_->RemoveAllChildren();
    inspectable_->BuildUI(layout_);
}


}
