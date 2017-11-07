#pragma once

#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>

namespace Urho3D
{

class TabBar : public BorderImage
{
    URHO3D_OBJECT(TabBar, BorderImage);

public:
    TabBar(Context* context);

    static void RegisterObject(Context* context);

    /// Set whether to fill the tab bar to its max size.
    void SetFill(bool fill);
    void SetTabBorder(const IntRect& tabBorder);
    void SetScrollSpeed(int scrollSpeed);

    Button* AddTab(const String& text);

    /// \see UIElement::OnResize
    void OnResize(const IntVector2& newSize, const IntVector2& delta) override;
    /// \see UIElement::OnWheel
    void OnWheel(int delta, int buttons, int qualifiers) override;
    /// \see UIElement::IsWheelHandler
    bool IsWheelHandler() const override { return true; }

private:
    /// Handle tab button clicked.
    void HandleTabClicked(StringHash eventType, VariantMap& eventData);
    /// Update tab bar offset.
    void UpdateOffset();

private:
    IntRect tabBorder_ = IntRect(8, 2, 8, 2);
    int scrollSpeed_ = 60;

    UIElement* filler_ = nullptr;

    PODVector<Button*> tabs_;

    int offset_ = 0;
};

}
