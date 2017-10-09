#pragma once

#include "../GenericUI/GenericUI.h"

namespace Urho3D
{

class Inspector : public Object
{
    URHO3D_OBJECT(Inspector, Object);

public:
    Inspector(AbstractMainWindow& mainWindow);

private:
    SharedPtr<GenericDialog> dialog_;
};

}
