#include "Inspector.h"

namespace Urho3D
{

Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockRight);
    dialog_->SetName("Inspector");
    AbstractScrollArea* scrollRegion = dialog_->CreateContent<AbstractScrollArea>();
    scrollRegion->SetDynamicWidth(true);
    AbstractLayout* layout = scrollRegion->CreateContent<AbstractLayout>();

    int row = 0;
    for (int i = 0; i < 50; ++i)
    {
        layout->CreateCellWidget<AbstractText>(row, 0)->SetText("Position").SetFixedWidth(false);
        AbstractLayout* nestedLayout1 = layout->CreateCellWidget<AbstractLayout>(row, 1);
        nestedLayout1->CreateCellWidget<AbstractText>(0, 0)->SetText("X");
        nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 1)->SetText("1");
        nestedLayout1->CreateCellWidget<AbstractText>(0, 2)->SetText("Y");
        nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 3)->SetText("2");
        nestedLayout1->CreateCellWidget<AbstractText>(0, 4)->SetText("Z");
        nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 5)->SetText("3");
        nestedLayout1->CreateCellWidget<AbstractText>(0, 6)->SetText("W");
        nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 7)->SetText("4");
        ++row;
        layout->CreateCellWidget<AbstractText>(row, 0)->SetText("Some long long long name").SetFixedWidth(false);
        layout->CreateCellWidget<AbstractLineEdit>(row, 1)->SetText("Some long long long edit");
        ++row;
        layout->CreateRowWidget<AbstractButton>(row)->SetText("Build");
        ++row;
        layout->CreateCellWidget<AbstractText>(row, 0)->SetText("Two Buttons").SetFixedWidth(false);
        AbstractLayout* nestedLayout2 = layout->CreateCellWidget<AbstractLayout>(row, 1);
        nestedLayout2->CreateCellWidget<AbstractButton>(0, 0)->SetText("1");
        nestedLayout2->CreateCellWidget<AbstractButton>(0, 1)->SetText("2");
        ++row;
    }

}

}
