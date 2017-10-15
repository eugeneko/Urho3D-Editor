#include "Inspector.h"

namespace Urho3D
{

VectorAttributeEditor::VectorAttributeEditor(Context* context, unsigned numComponents)
    : AttributeEditor(context)
    , numComponents_(numComponents)
{
    assert(numComponents >= 1 && numComponents <= 4);
}

void VectorAttributeEditor::BuildUI(AbstractLayout* layout, unsigned& row, unsigned column)
{
    internalLayout_ = layout->CreateCell<AbstractLayout>(row, column);
    unsigned cell = 0;
    const char* labels[] = { "X", "Y", "Z", "W" };
    for (unsigned i = 0; i < numComponents_; ++i)
    {
        AbstractText* componentLabel = internalLayout_->CreateCell<AbstractText>(0, cell++);
        componentLabel->SetText(labels[i]);
        AbstractLineEdit* componentValue = internalLayout_->CreateCell<AbstractLineEdit>(0, cell++);
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
        AbstractText* attributeNameText = layout->CreateCell<AbstractText>(row, 0);
        attributeNameText->SetText(attributeInfo.name_);

        // Gather values
        values.Clear();
        for (Serializable* object : objects_)
            values.Push(object->GetAttribute(i));

        if (attributeInfo.type_ == VAR_VECTOR3)
        {
            auto editor = MakeShared<VectorAttributeEditor>(context_, 3);
            editor->BuildUI(layout, row, 1);
            editor->SetValues(values);
            attributes_.Push(editor);
        }
        ++row;
    }
    layout->CreateRow<AbstractDummyWidget>(row);
}

//////////////////////////////////////////////////////////////////////////
void MultiplePanelInspector::AddPanel(Inspectable* panel)
{
    panels_.Push(SharedPtr<Inspectable>(panel));
}

void MultiplePanelInspector::BuildUI(AbstractLayout* layout)
{
//     for (unsigned i = 0; i < panels_.Size(); ++i)
//     {
//         AbstractCollapsiblePanel* collapsiblePanel
//         layout->
//     }
//     layout->
//
//     for ()
//     {
//     }
}

//////////////////////////////////////////////////////////////////////////
Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockRight);
    dialog_->SetName("Inspector");
    scrollRegion_ = dialog_->CreateContent<AbstractScrollArea>();
    layout_ = scrollRegion_->CreateContent<AbstractLayout>();
    return;
    int numPanels = 5;
    for (int j = 0; j < numPanels; ++j)
    {
        AbstractCollapsiblePanel* panel = layout_->CreateRow<AbstractCollapsiblePanel>(j);
        panel->SetExpanded(true);

        AbstractLayout* headerPrefix = panel->CreateHeaderPrefix<AbstractLayout>();
        headerPrefix->CreateCell<AbstractCheckBox>(0, 0);
//         panel->CreateHeaderPrefix<AbstractCheckBox>();
        panel->SetHeaderText("Panel" + String(j));
//         panel->CreateHeaderSuffix<AbstractButton>()->SetText("Butt");;
        AbstractLayout* headerSuffix = panel->CreateHeaderSuffix<AbstractLayout>();
        headerSuffix->CreateCell<AbstractButton>(0, 2)->SetText("Butt");
        AbstractLayout* bodyLayout = panel->CreateBody<AbstractLayout>();
        int row = 0;
        for (int i = 0; i < 10; ++i)
        {
            bodyLayout->CreateCell<AbstractText>(row, 0)->SetText("Position");
            AbstractLayout* nestedLayout1 = bodyLayout->CreateCell<AbstractLayout>(row, 1);
            nestedLayout1->CreateCell<AbstractText>(0, 0)->SetText("X");
            nestedLayout1->CreateCell<AbstractLineEdit>(0, 1)->SetText("1");
            nestedLayout1->CreateCell<AbstractText>(0, 2)->SetText("Y");
            nestedLayout1->CreateCell<AbstractLineEdit>(0, 3)->SetText("2");
            nestedLayout1->CreateCell<AbstractText>(0, 4)->SetText("Z");
            nestedLayout1->CreateCell<AbstractLineEdit>(0, 5)->SetText("3");
            nestedLayout1->CreateCell<AbstractText>(0, 6)->SetText("W");
            nestedLayout1->CreateCell<AbstractLineEdit>(0, 7)->SetText("4");
            ++row;
            bodyLayout->CreateCell<AbstractText>(row, 0)->SetText("Some long long long name");
            bodyLayout->CreateCell<AbstractLineEdit>(row, 1)->SetText("Some long long long edit");
            ++row;
            bodyLayout->CreateRow<AbstractButton>(row)->SetText("Build");
            ++row;
            bodyLayout->CreateCell<AbstractText>(row, 0)->SetText("Two Buttons");
            AbstractLayout* nestedLayout2 = bodyLayout->CreateCell<AbstractLayout>(row, 1);
            nestedLayout2->CreateCell<AbstractButton>(0, 0)->SetText("1");
            nestedLayout2->CreateCell<AbstractButton>(0, 1)->SetText("2");
            nestedLayout2->CreateCell<AbstractCheckBox>(0, 2)->SetChecked(true);
            ++row;
        }
    }

    layout_->CreateRow<AbstractDummyWidget>(numPanels);

}

void Inspector::SetInspectable(Inspectable* inspectable)
{
    inspectable_ = inspectable;
    layout_->RemoveAllChildren();
    inspectable_->BuildUI(layout_);
}


}
