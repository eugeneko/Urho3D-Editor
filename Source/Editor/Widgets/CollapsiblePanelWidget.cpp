#include "CollapsiblePanelWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QScrollArea>
#include <QToolButton>

namespace Urho3DEditor
{

CollapsiblePanelWidget::CollapsiblePanelWidget(const QString& title /*= ""*/, bool expanded /*= false*/,
    QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , mainLayout_(new QGridLayout(this))
    , toggleButton_(new QToolButton(this))
    , headerLine_(new QFrame(this))
    , body_(new QScrollArea(this))
{
    toggleButton_->setStyleSheet("QToolButton {border: none;}");
    toggleButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton_->setArrowType(Qt::ArrowType::RightArrow);
    toggleButton_->setText(title);
    toggleButton_->setCheckable(true);
    toggleButton_->setChecked(false);

    headerLine_->setFrameShape(QFrame::HLine);
    headerLine_->setFrameShadow(QFrame::Sunken);
    headerLine_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    body_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    body_->setMaximumHeight(0);
    body_->setMinimumHeight(0);

    mainLayout_->setVerticalSpacing(0);
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    mainLayout_->addWidget(toggleButton_, 0, 0, 1, 1, Qt::AlignLeft);
    mainLayout_->addWidget(headerLine_, 0, 2, 1, 1);
    mainLayout_->addWidget(body_, 1, 0, 1, 3);
    setLayout(mainLayout_);

    connect(toggleButton_, SIGNAL(clicked(bool)), this, SLOT(SetCollapsed(bool)));
    if (expanded)
        toggleButton_->click();
}

void CollapsiblePanelWidget::SetContentLayout(QLayout* contentLayout)
{
    body_->setLayout(contentLayout);
    SetCollapsed(toggleButton_->isChecked());
}

void CollapsiblePanelWidget::SetCollapsed(bool checked)
{
    const int collapsedHeight = sizeHint().height() - body_->maximumHeight();
    const int contentHeight = body_->layout() ? body_->layout()->sizeHint().height() : 0;

    toggleButton_->setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    setMinimumHeight(checked ? collapsedHeight + contentHeight : collapsedHeight);
    setMaximumHeight(checked ? collapsedHeight + contentHeight : collapsedHeight);
    body_->setMaximumHeight(checked ? contentHeight : 0);
}

}
