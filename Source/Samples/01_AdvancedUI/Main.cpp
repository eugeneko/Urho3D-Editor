#include "../../Library/AdvancedUI/MenuBar.h"

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/DebugNew.h>

using namespace Urho3D;

class UrhoEditorApplication : public Application
{
    URHO3D_OBJECT(UrhoEditorApplication, Application);

public:
    UrhoEditorApplication(Context* context) : Application(context) { }

    virtual void Start() override
    {
        Application::Start();

        Input* input = GetSubsystem<Input>();
        UI* ui = GetSubsystem<UI>();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        input->SetMouseVisible(true);
        UIElement* uiRoot = ui->GetRoot();
        uiRoot->SetDefaultStyle(style);

        CreateUI();
        UpdateElements();
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

        // Create menu bar
        menuBar_ = new MenuBar(context_);
        menuBar_->SetLayout(LM_HORIZONTAL);
        uiRoot->AddChild(menuBar_);

        // Create menus
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
    }

    void UpdateElements()
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        menuBar_->SetSize(graphics->GetWidth(), menuBar_->GetEffectiveMinSize().y_);
    }

    MenuBar* menuBar_ = nullptr;

};

URHO3D_DEFINE_APPLICATION_MAIN(UrhoEditorApplication)
