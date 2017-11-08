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

        SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(SampleApplication, HandleResized));
    }

private:
    void HandleResized(StringHash /*eventType*/, VariantMap& /*eventData*/)
    {
        UpdateElements();
    }

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

        // Create tab bar
        tabBar_ = mainUi_->CreateChild<TabBar>();
        tabBar_->SetStyle("Menu");
        tabBar_->SetHoverOffset(IntVector2::ZERO);
        tabBar_->SetExpand(true);
        tabBar_->AddTab("Document1");
        tabBar_->AddTab("Document2");
        tabBar_->AddTab("Document3");
        tabBar_->AddTab("Document4");
        tabBar_->AddTab("Document With Very Very Very Long Title 1");
        tabBar_->AddTab("Document With Very Very Very Long Title 2");
        tabBar_->AddTab("Document With Very Very Very Long Title 3");
        tabBar_->AddTab("Document With Very Very Very Long Title 4");

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
            text->SetStyleAuto();
            text->SetText(title);
        };

        createDock(DL_LEFT, "Hierarchy");
        createDock(DL_RIGHT, "Inspector");
        createDock(DL_BOTTOM, "Resource Browser");
        createDock(DL_BOTTOM, "Log");

        /*document_ = mainUi_->CreateChild<UIElement>();
        document_->SetLayout(LM_HORIZONTAL);

        SplitView* splitView = document_->CreateChild<SplitView>();
        splitView->SetDefaultLineStyle();
        splitView->SetSplit(SPLIT_VERTICAL);
        splitView->SetRelativePosition(0.5f);
        {
            SplitView* leftSplit = splitView->CreateFirstChild<SplitView>();
            leftSplit->SetDefaultLineStyle();
            leftSplit->SetSplit(SPLIT_HORIZONTAL);
            leftSplit->SetFixedPosition(100, SA_BEGIN);
            {
                SplitView* topSplit = leftSplit->CreateFirstChild<SplitView>();
                topSplit->SetDefaultLineStyle();
                topSplit->SetSplit(SPLIT_HORIZONTAL);
                topSplit->SetRelativePosition(0.3f);
            }
            {
                SplitView* bottomSplit = leftSplit->CreateSecondChild<SplitView>();
                bottomSplit->SetDefaultLineStyle();
                bottomSplit->SetSplit(SPLIT_VERTICAL);
                bottomSplit->SetFixedPosition(100, SA_BEGIN);
            }
        }
        {
            SplitView* rightSplit = splitView->CreateSecondChild<SplitView>();
            rightSplit->SetDefaultLineStyle();
            rightSplit->SetSplit(SPLIT_HORIZONTAL);
            rightSplit->SetFixedPosition(100, SA_END);
            {
                SplitView* topSplit = rightSplit->CreateFirstChild<SplitView>();
                topSplit->SetDefaultLineStyle();
                topSplit->SetSplit(SPLIT_VERTICAL);
                topSplit->SetFixedPosition(100, SA_END);
            }
            {
                SplitView* bottomSplit = rightSplit->CreateSecondChild<SplitView>();
                bottomSplit->SetDefaultLineStyle();
                bottomSplit->SetSplit(SPLIT_VERTICAL);
                bottomSplit->SetRelativePosition(0.3f);
            }
        }*/
    }

    void UpdateElements()
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        mainUi_->SetFixedSize(graphics->GetWidth(), graphics->GetHeight());
        menuBar_->SetFixedWidth(graphics->GetWidth());
        menuBar_->SetMaxHeight(menuBar_->GetEffectiveMinSize().y_);
        tabBar_->SetMaxHeight(tabBar_->GetEffectiveMinSize().y_);

    }

    UIElement* mainUi_ = nullptr;
    MenuBar* menuBar_ = nullptr;
    TabBar* tabBar_ = nullptr;
    DockView* document_ = nullptr;
};

URHO3D_DEFINE_APPLICATION_MAIN(SampleApplication)
