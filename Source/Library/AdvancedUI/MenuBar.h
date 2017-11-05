#pragma once

#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Menu.h>

namespace Urho3D
{

class MenuBar : public BorderImage
{
    URHO3D_OBJECT(MenuBar, BorderImage);

public:
    MenuBar(Context* context);

    static void RegisterObject(Context* context);

    void SetMenuLayout(int spacing, const IntRect& border);
    void SetPopupLayout(int spacing, const IntRect& border);
    Menu* CreateMenu(const String& text, int key, int quals, Menu* parent = nullptr);
    Menu* CreateMenu(const String& text, Menu* parent = nullptr);
    Menu* CreateMenu(Menu* parent = nullptr);

private:
    void UpdateMenuAccelText(Menu* menu, const String& text);
    void UpdateMenuSize(Menu* menu);

private:
    int menuSpacing_ = 0;
    IntRect menuBorder_ = IntRect(8, 2, 8, 2);
    int popupSpacing_ = 1;
    IntRect popupBorder_ = IntRect(2, 6, 2, 6);
};

}
