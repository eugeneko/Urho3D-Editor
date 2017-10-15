#pragma once

#include "../GenericUI/GenericUI.h"

namespace Urho3D
{

class Inspectable : public Object
{
    URHO3D_OBJECT(Inspectable, Object);

public:


};

class SerializableInspector : public Object
{
    URHO3D_OBJECT(SerializableInspector, Object);

public:

private:

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
