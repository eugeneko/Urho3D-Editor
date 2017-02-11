#include "CollapsiblePanelWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QScrollArea>
#include <QToolButton>

namespace Urho3DEditor
{

CollapsiblePanelWidget::CollapsiblePanelWidget(const QString& title /*= ""*/,
    bool expanded /*= false*/, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
    , mainLayout_(new QGridLayout(this))
    , toggleButton_(new QToolButton(this))
    , headerLine_(new QFrame(this))
    , body_(new QScrollArea(this))
    , expanded_(expanded)
    , headerHeight_(0)
    , bodyHeight_(0)
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

    body_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    body_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    body_->setWidgetResizable(true);

    mainLayout_->setVerticalSpacing(0);
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    mainLayout_->addWidget(toggleButton_, 0, 0, 1, 1, Qt::AlignLeft);
    mainLayout_->addWidget(headerLine_, 0, 2, 1, 1);
    mainLayout_->addWidget(body_, 1, 0, 1, 3);
    setLayout(mainLayout_);

    headerHeight_ = mainLayout_->sizeHint().height();

    connect(toggleButton_, SIGNAL(clicked(bool)), this, SLOT(SetCollapsed(bool)));
    UpdateSizes();
}

void CollapsiblePanelWidget::SetCentralWidget(QWidget* widget)
{
    body_->setWidget(widget);
    bodyHeight_ = widget->sizeHint().height();
    UpdateSizes();
}

void CollapsiblePanelWidget::SetCollapsed(bool checked)
{
    expanded_ = checked;
    UpdateSizes();
}

void CollapsiblePanelWidget::UpdateSizes()
{
    toggleButton_->setArrowType(expanded_ ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    setMinimumHeight(expanded_ ? headerHeight_ + bodyHeight_ : headerHeight_);
    setMaximumHeight(expanded_ ? headerHeight_ + bodyHeight_ : headerHeight_);
    body_->setMaximumHeight(expanded_ ? bodyHeight_ : 0);
}

}
