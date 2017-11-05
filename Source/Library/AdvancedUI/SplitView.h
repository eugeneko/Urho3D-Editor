#pragma once

#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/UIElement.h>

namespace Urho3D
{

enum SplitOrientation
{
    SPLIT_HORIZONTAL,
    SPLIT_VERTICAL
};

class SplitLine : public BorderImage
{
    URHO3D_OBJECT(SplitLine, BorderImage);

public:
    SplitLine(Context* context);

    static void RegisterObject(Context* context);

    void SetThreshold(int threshold) { threshold_ = threshold; }
    void SetOrientation(SplitOrientation orientation);

    /// \see UIElement::OnHover
    void OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) override;
    /// \see UIElement::OnDragBegin
    void OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor) override;
    /// \see UIElement::OnDragMove
    void OnDragMove(const IntVector2& position, const IntVector2& screenPosition, const IntVector2& deltaPos, int buttons, int qualifiers, Cursor* cursor) override;
    /// \see UIElement::OnDragEnd
    void OnDragEnd(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor) override;
    /// \see UIElement::OnDragCancel
    void OnDragCancel(const IntVector2& position, const IntVector2& screenPosition, int dragButtons, int buttons, Cursor* cursor) override;

private:
    IntVector2 dragBeginPosition_;
    SplitOrientation orientation_ = SPLIT_VERTICAL;
    int threshold_ = 8;
};

enum SplitAnchor
{
    SA_BEGIN,
    SA_END
};

class SplitView : public UIElement
{
    URHO3D_OBJECT(SplitView, UIElement);

public:
    SplitView(Context* context);

    static void RegisterObject(Context* context);

    void SetDefaultLineStyle();
    void SetThreshold(int threshold);
    void SetSplit(SplitOrientation orientation);
    void SetRelativePosition(float relativePosition);
    void SetFixedPosition(int offset, SplitAnchor anchor);

    void SetFirstChild(UIElement* child);
    void SetSecondChild(UIElement* child);

    /// Create and add a first child element and return it.
    UIElement* CreateFirstChild(StringHash type, const String& name = String::EMPTY);
    /// Create and add a second child element and return it.
    UIElement* CreateSecondChild(StringHash type, const String& name = String::EMPTY);
    /// Create and add a first child element and return it (template).
    template <class T> T* CreateFirstChild(const String& name = String::EMPTY);
    /// Create and add a second child element and return it (template).
    template <class T> T* CreateSecondChild(const String& name = String::EMPTY);

    /// Get split line element.
    BorderImage* GetSplitLine() const { return splitLine_; }

    void OnResize(const IntVector2& newSize, const IntVector2& delta) override;

private:
    void HandleSplitLineMoved(StringHash eventType, VariantMap& eventData);
    void UpdateSplitLine();
    void UpdateChildren();

    int ElementToLinePosition(int pos) { return pos + threshold_ / 2; }
    int LineToElementPosition(int pos, int parentSize) { return Clamp(pos - threshold_ / 2, 0, parentSize - threshold_); }

private:
    SplitLine* splitLine_ = nullptr;
    SplitOrientation orientation_ = SPLIT_VERTICAL;
    int threshold_ = 0;

    /// Whether to use relative position of split line.
    bool useRelativePosition_ = true;
    /// Relative position of the split line.
    float relativePosition_ = 0.5f;
    /// Fixed split line anchor.
    SplitAnchor anchor_ = SA_BEGIN;
    /// Fixed split line offset.
    int offset_ = 0;

    /// First child.
    UIElement* firstChild_ = nullptr;
    /// Second child.
    UIElement* secondChild_ = nullptr;

};

template <class T> T* SplitView::CreateSecondChild(const String& name)
{
    return static_cast<T*>(CreateSecondChild(T::GetTypeStatic(), name));
}

template <class T> T* SplitView::CreateFirstChild(const String& name)
{
    return static_cast<T*>(CreateFirstChild(T::GetTypeStatic(), name));
}

}
