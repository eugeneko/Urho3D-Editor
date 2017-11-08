#include "../../Library/AdvancedUI/MenuBar.h"
#include "../../Library/AdvancedUI/SplitView.h"
#include "../../Library/AdvancedUI/TabBar.h"
#include "../../Library/AdvancedUI/StackView.h"
#include "../../Library/AdvancedUI/DockView.h"

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

#include <Urho3D/UI/Text.h>

#include <Urho3D/DebugNew.h>

using namespace Urho3D;

class SampleApplication : public Application
{
    URHO3D_OBJECT(SampleApplication, Application);

public:
    SampleApplication(Context* context) : Application(context) { }

    virtual void Start() override
    {
        Application::Start();

        Input* input = GetSubsystem<Input>();
        input->SetMouseVisible(true);
        UI* ui = GetSubsystem<UI>();

        MenuBar::RegisterObject(context_);
        SplitLine::RegisterObject(context_);
        SplitView::RegisterObject(context_);
        TabButton::RegisterObject(context_);
        TabBar::RegisterObject(context_);
        DockView::RegisterObject(context_);
        StackView::RegisterObject(context_);

        CreateUI();
        UpdateElements();

        SubscribeToEvent(E_SCREENMODE,
            [=](StringHash /*eventType*/, VariantMap& /*eventData*/)
        {
            UpdateElements();
        });
    }

private:
    void CreateUI()
    {
        UI* ui = GetSubsystem<UI>();
        UIElement* uiRoot = ui->GetRoot();

        // Setup UI
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        uiRoot->SetDefaultStyle(style);
        ui->SetNonFocusedMouseWheel(true);

        // Setup cursor
        auto cursor = MakeShared<Cursor>(context_);
        cursor->SetStyleAuto();
        ui->SetCursor(cursor);

        // Setup main UI
        mainUi_ = uiRoot->CreateChild<UIElement>();
        mainUi_->SetLayout(LM_VERTICAL);

        // Create menu bar
        menuBar_ = mainUi_->CreateChild<MenuBar>();
        menuBar_->SetLayout(LM_HORIZONTAL);
        menuBar_->SetStyle("Menu");

        // Create menu
        if (Menu* menuFile = menuBar_->CreateMenu("File"))
        {
            menuBar_->CreateMenu("Open", KEY_O, QUAL_CTRL, menuFile);
            menuBar_->CreateMenu("Save", KEY_S, QUAL_CTRL, menuFile);
            if (Menu* menuFileRecent = menuBar_->CreateMenu("Recent", menuFile))
            {
                menuBar_->CreateMenu("File 1", menuFileRecent);
                menuBar_->CreateMenu("File 2", menuFileRecent);
                menuBar_->CreateMenu("File 3", menuFileRecent);
            }
        }
        if (Menu* menuEdit = menuBar_->CreateMenu("Edit"))
        {
            menuBar_->CreateMenu("Cut", KEY_X, QUAL_CTRL, menuEdit);
            menuBar_->CreateMenu("Copy", KEY_C, QUAL_CTRL, menuEdit);
            menuBar_->CreateMenu("Paste", KEY_P, QUAL_CTRL, menuEdit);
            menuBar_->CreateMenu("Delete", KEY_DELETE, 0, menuEdit);
        }
        menuBar_->CreateMenu("About");

        // Create document content
        document_ = mainUi_->CreateChild<DockView>();
        document_->SetDefaultSplitStyle();
        document_->SetDefaultTabBarStyle();

        auto createDock = [=](DockLocation location, const String& title)
        {
            Button* button = new Button(context_);
            Text* text = button->CreateChild<Text>();
            document_->AddDock(location, title, button);
            button->SetStyleAuto();
            button->SetLayout(LM_HORIZONTAL, 0, IntRect(2, 2, 2, 2));
            text->SetStyleAuto();
            text->SetText(title);
        };

        createDock(DL_LEFT, "Hierarchy");
        createDock(DL_RIGHT, "Inspector");
        createDock(DL_BOTTOM, "Resource Browser");
        createDock(DL_BOTTOM, "Log");

        // Setup center element
        UIElement* centerElement = document_->GetCentralElement();
        centerElement->SetLayout(LM_VERTICAL);
        SubscribeToEvent(centerElement, E_RESIZED,
            [=](StringHash eventType, VariantMap& eventData)
        {
            UpdateCentralElement();
        });

        // Create tab bar
        tabBar_ = centerElement->CreateChild<TabBar>();
        tabBar_->SetStyle("Menu");
        tabBar_->SetHoverOffset(IntVector2::ZERO);
        tabBar_->SetExpand(true);
        tabBar_->AddTab("Document1");
        tabBar_->AddTab("Document2");
        tabBar_->AddTab("Document3");
        tabBar_->AddTab("Document4");
        tabBar_->AddTab("Document With Very Very Very Long Title");
    }

    void UpdateElements()
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        mainUi_->SetFixedSize(graphics->GetWidth(), graphics->GetHeight());
        menuBar_->SetFixedWidth(graphics->GetWidth());
        menuBar_->SetMaxHeight(menuBar_->GetEffectiveMinSize().y_);
    }
    void UpdateCentralElement()
    {
    }

    UIElement* mainUi_ = nullptr;
    MenuBar* menuBar_ = nullptr;
    DockView* document_ = nullptr;
    TabBar* tabBar_ = nullptr;
};

URHO3D_DEFINE_APPLICATION_MAIN(SampleApplication)
