#include "../../Library/Editor/StandardEditor.h"
#include "../../Library/AbstractUI/Urho/UrhoUI.h"
#include "../../Library/AbstractUI/Qt/QtUI.h"

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/DebugNew.h>

using namespace Urho3D;

int QtEditorMain()
{
    static int argcStub = 0;
    static char* argvStub[] = { nullptr };
    QApplication::setStyle("Fusion");
    QApplication applicaton(argcStub, argvStub);
    QtMainWindow mainWindow(applicaton);
    StandardEditor defaultEditor(&mainWindow, false);
    mainWindow.showMaximized();
    return applicaton.exec();
}

//////////////////////////////////////////////////////////////////////////
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
        ui->GetRoot()->SetDefaultStyle(style);

        mainWindow_ = MakeShared<UrhoMainWindow>(context_);
        editor_ = MakeShared<StandardEditor>(mainWindow_, false);
    }

private:
    SharedPtr<UrhoMainWindow> mainWindow_;
    SharedPtr<StandardEditor> editor_;
};


URHO3D_DEFINE_MAIN(QtEditorMain())
// URHO3D_DEFINE_APPLICATION_MAIN(UrhoEditorApplication)
