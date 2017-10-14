#include "Inspector.h"

namespace Urho3D
{

Inspector::Inspector(AbstractMainWindow& mainWindow)
    : Object(mainWindow.GetContext())
{
    dialog_ = mainWindow.AddDialog(DialogLocationHint::DockRight);
    dialog_->SetName("Inspector");
    AbstractScrollArea* scrollRegion = dialog_->CreateContent<AbstractScrollArea>();
    //scrollRegion->SetDynamicWidth(true);
    AbstractLayout* layout = scrollRegion->CreateContent<AbstractLayout>();

    int numPanels = 5;
    for (int j = 0; j < numPanels; ++j)
    {
        AbstractCollapsiblePanel* panel = layout->CreateRowWidget<AbstractCollapsiblePanel>(j);
        panel->SetExpanded(true);

        AbstractLayout* headerPrefix = panel->CreateHeaderPrefix<AbstractLayout>();
        headerPrefix->CreateCellWidget<AbstractCheckBox>(0, 0);
//         panel->CreateHeaderPrefix<AbstractCheckBox>();
        panel->SetHeaderText("Panel" + String(j));
//         panel->CreateHeaderSuffix<AbstractButton>()->SetText("Butt");;
        AbstractLayout* headerSuffix = panel->CreateHeaderSuffix<AbstractLayout>();
        headerSuffix->CreateCellWidget<AbstractButton>(0, 2)->SetText("Butt");
        AbstractLayout* bodyLayout = panel->CreateBody<AbstractLayout>();
        int row = 0;
        for (int i = 0; i < 10; ++i)
        {
            bodyLayout->CreateCellWidget<AbstractText>(row, 0)->SetText("Position");
            AbstractLayout* nestedLayout1 = bodyLayout->CreateCellWidget<AbstractLayout>(row, 1);
            nestedLayout1->CreateCellWidget<AbstractText>(0, 0)->SetText("X");
            nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 1)->SetText("1");
            nestedLayout1->CreateCellWidget<AbstractText>(0, 2)->SetText("Y");
            nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 3)->SetText("2");
            nestedLayout1->CreateCellWidget<AbstractText>(0, 4)->SetText("Z");
            nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 5)->SetText("3");
            nestedLayout1->CreateCellWidget<AbstractText>(0, 6)->SetText("W");
            nestedLayout1->CreateCellWidget<AbstractLineEdit>(0, 7)->SetText("4");
            ++row;
            bodyLayout->CreateCellWidget<AbstractText>(row, 0)->SetText("Some long long long name");
            bodyLayout->CreateCellWidget<AbstractLineEdit>(row, 1)->SetText("Some long long long edit");
            ++row;
            bodyLayout->CreateRowWidget<AbstractButton>(row)->SetText("Build");
            ++row;
            bodyLayout->CreateCellWidget<AbstractText>(row, 0)->SetText("Two Buttons");
            AbstractLayout* nestedLayout2 = bodyLayout->CreateCellWidget<AbstractLayout>(row, 1);
            nestedLayout2->CreateCellWidget<AbstractButton>(0, 0)->SetText("1");
            nestedLayout2->CreateCellWidget<AbstractButton>(0, 1)->SetText("2");
            nestedLayout2->CreateCellWidget<AbstractCheckBox>(0, 2)->SetChecked(true);
            ++row;
        }
    }

    layout->CreateRowWidget<AbstractDummyWidget>(numPanels);

}

}
