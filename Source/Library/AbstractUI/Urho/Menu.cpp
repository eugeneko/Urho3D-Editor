#include "Menu.h"

namespace Urho3D
{

MenuWithPopupCallback::MenuWithPopupCallback(Context* context)
    : Menu(context)
{
}

void MenuWithPopupCallback::OnShowPopup()
{
    if (onShowPopup_)
        onShowPopup_();
}

}
