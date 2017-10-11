#include "Inspector.h"

namespace Urho3D
{

Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockLeft);
    dialog_->SetName("Inspector");
    AbstractScrollRegion* scrollRegion = dialog_->CreateBodyWidget<AbstractScrollRegion>();
    scrollRegion->SetDynamicWidth(true);
    AbstractLayout* layout = scrollRegion->CreateContent<AbstractLayout>();

}

}
