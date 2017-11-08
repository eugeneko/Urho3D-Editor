#pragma once

#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>

namespace Urho3D
{

// #TODO Move to header
URHO3D_EVENT(E_TABSELECTED, TabSelected)
{
    URHO3D_PARAM(P_ELEMENT, Element);
    URHO3D_PARAM(P_TAB, Tab);
}

class TabButton : public Button
{
    URHO3D_OBJECT(TabButton, Button);

public:
    TabButton(Context* context);

    static void RegisterObject(Context* context);

    /// \see UIElement::Update
    void Update(float timeStep) override;
    /// \see UIElement::OnDragBegin
    void OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) override;
    /// \see UIElement::OnDragEnd
    void OnDragEnd(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor) override;
    /// \see UIElement::OnDragCancel
    void OnDragCancel(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor) override;

private:
    bool isDragging_ = false;
};

class TabBar : public BorderImage
{
    URHO3D_OBJECT(TabBar, BorderImage);

public:
    TabBar(Context* context);

    static void RegisterObject(Context* context);

    /// Set whether to expand the tab bar to its max size.
    void SetExpand(bool expand);
    void SetTabBorder(const IntRect& tabBorder);
    void SetScrollSpeed(int scrollSpeed);

    /// Construct default tab button. The tab is not added to the tab bar.
    TabButton* ConstructDefaultTab(const String& text) const;
    void AddTab(TabButton* tab);
    TabButton* AddTab(const String& text);
    void RemoveTab(TabButton* tab);
    void SwitchToTab(TabButton* tab);
    void ReorderTab(TabButton* tab, unsigned index);

    /// Get number of tabs.
    unsigned GetNumTabs() const { return tabs_.Size(); }

    /// \see UIElement::OnResize
    void OnResize(const IntVector2& newSize, const IntVector2& delta) override;
    /// \see UIElement::OnWheel
    void OnWheel(int delta, int buttons, int qualifiers) override;
    /// \see UIElement::IsWheelHandler
    bool IsWheelHandler() const override { return true; }

private:
    /// Handle layout updated.
    void HandleLayoutUpdated(StringHash eventType, VariantMap& eventData);
    /// Handle tab button clicked.
    void HandleTabClicked(StringHash eventType, VariantMap& eventData);
    /// Update tab bar offset.
    void UpdateOffset();

private:
    IntRect tabBorder_ = IntRect(8, 2, 8, 2);
    int scrollSpeed_ = 60;

    UIElement* filler_ = nullptr;

    PODVector<TabButton*> tabs_;
    TabButton* activeTab_ = nullptr;

    int offset_ = 0;
    unsigned dragBeginIndex_ = 0;
};

}
