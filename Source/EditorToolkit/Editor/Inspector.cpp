#include "Inspector.h"

namespace Urho3D
{

Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockLeft);
    dialog_->SetName("Inspector");
    dialog_->CreateBodyWidget<AbstractLayout>();
}

}
