#pragma once

#include <Urho3D/UI/Menu.h>
#include <functional>

namespace Urho3D
{

class MenuWithPopupCallback : public Menu
{
public:
    std::function<void()> onShowPopup_;

public:
    MenuWithPopupCallback(Context* context);

    virtual void OnShowPopup() override;
};

}
