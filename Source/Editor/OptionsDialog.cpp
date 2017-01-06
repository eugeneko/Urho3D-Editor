#include "OptionsDialog.h"
#include "Configuration.h"
#include <Urho3D/Math/MathDefs.h>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>

namespace Urho3DEditor
{

class ConfigurationVariableImpl
{
public:
    /// Get widget.
    virtual QWidget* GetWidget() const = 0;
    /// Get value.
    virtual QVariant GetValue() const = 0;
    /// Set value.
    virtual void SetValue(const QVariant& value) const = 0;
};

class VoidVariableImpl : public ConfigurationVariableImpl
{
public:
    /// Get widget.
    virtual QWidget* GetWidget() const override { return nullptr; }
    /// Get value.
    virtual QVariant GetValue() const override { return QVariant(); }
    /// Set value.
    virtual void SetValue(const QVariant& /*value*/) const override {}
};

class BoolVariableImpl : public ConfigurationVariableImpl
{
public:
    /// Construct.
    BoolVariableImpl() : widget_(new QCheckBox()) { }

    /// Get widget.
    virtual QWidget* GetWidget() const override { return widget_.data(); }
    /// Get value.
    virtual QVariant GetValue() const override { return widget_->isChecked(); }
    /// Set value.
    virtual void SetValue(const QVariant& value) const override { widget_->setChecked(value.toBool()); }

private:
    /// Widget.
    QScopedPointer<QCheckBox> widget_;

};

class StringVariableImpl : public ConfigurationVariableImpl
{
public:
    /// Construct.
    StringVariableImpl(QValidator* validator = nullptr)
        : widget_(new QLineEdit())
    {
        widget_->setValidator(validator);
        validator->setParent(widget_.data());
    }

    /// Get widget.
    virtual QWidget* GetWidget() const override { return widget_.data(); }
    /// Get value.
    virtual QVariant GetValue() const override { return widget_->text(); }
    /// Set value.
    virtual void SetValue(const QVariant& value) const override { widget_->setText(value.toString()); }

protected:
    /// Widget.
    QScopedPointer<QLineEdit> widget_;

};

class IntegerVariableImpl : public StringVariableImpl
{
public:
    /// Construct.
    IntegerVariableImpl(bool isSigned)
        : StringVariableImpl(isSigned
            ? new QIntValidator(Urho3D::M_MIN_INT, Urho3D::M_MAX_INT)
            : new QIntValidator(0, Urho3D::M_MAX_INT))
    {}

    /// Get value.
    virtual QVariant GetValue() const override
    {
        return widget_->text().toInt();
    }
    /// Set value.
    virtual void SetValue(const QVariant& value) const override
    {
        widget_->setText(QString::number(value.toInt()));
    }

};

class DoubleVariableImpl : public StringVariableImpl
{
public:
    /// Construct.
    DoubleVariableImpl() : StringVariableImpl(new QDoubleValidator()) {}

    /// Get value.
    virtual QVariant GetValue() const override
    {
        return widget_->text().toDouble();
    }
    /// Set value.
    virtual void SetValue(const QVariant& value) const override
    {
        widget_->setText(QString::number(value.toDouble()));
    }

};

class EnumVariableImpl : public ConfigurationVariableImpl
{
public:
    /// Construct.
    EnumVariableImpl(const QVariant& decoration)
        : widget_(new QComboBox)
    {
        widget_->addItems(decoration.toStringList());
    }

    /// Get widget.
    virtual QWidget* GetWidget() const override { return widget_.data(); }
    /// Get value.
    virtual QVariant GetValue() const override { return widget_->currentIndex(); }
    /// Set value.
    virtual void SetValue(const QVariant& value) const override { widget_->setCurrentIndex(value.toInt()); }

private:
    /// Widget.
    QScopedPointer<QComboBox> widget_;

};

ConfigurationVariableImpl* CreateVariable(QVariant::Type type, const QVariant& decoration)
{
    switch (type)
    {
    case QVariant::String:
        return new StringVariableImpl();

    case QVariant::Bool:
        return new BoolVariableImpl();

    case QVariant::Int:
    case QVariant::LongLong:
        if (decoration.type() == QVariant::StringList)
            return new EnumVariableImpl(decoration);
        else
            return new IntegerVariableImpl(true);

    case QVariant::UInt:
    case QVariant::ULongLong:
        if (decoration.type() == QVariant::StringList)
            return new EnumVariableImpl(decoration);
        else
            return new IntegerVariableImpl(false);

    case QVariant::Double:
        return new DoubleVariableImpl();

    case QVariant::Invalid:
    case QVariant::Char:
    case QVariant::Map:
    case QVariant::List:
    case QVariant::StringList:
    case QVariant::ByteArray:
    case QVariant::BitArray:
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::Rect:
    case QVariant::RectF:
    case QVariant::Size:
    case QVariant::SizeF:
    case QVariant::Line:
    case QVariant::LineF:
    case QVariant::Point:
    case QVariant::PointF:
    case QVariant::RegExp:
    case QVariant::RegularExpression:
    case QVariant::Hash:
    case QVariant::EasingCurve:
    case QVariant::Uuid:
    case QVariant::ModelIndex:
    case QVariant::PersistentModelIndex:
    case QVariant::Font:
    case QVariant::Pixmap:
    case QVariant::Brush:
    case QVariant::Color:
    case QVariant::Palette:
    case QVariant::Image:
    case QVariant::Polygon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::KeySequence:
    case QVariant::Pen:
    case QVariant::TextLength:
    case QVariant::TextFormat:
    case QVariant::Matrix:
    case QVariant::Transform:
    case QVariant::Matrix4x4:
    case QVariant::Vector2D:
    case QVariant::Vector3D:
    case QVariant::Vector4D:
    case QVariant::Quaternion:
    case QVariant::PolygonF:
    case QVariant::Icon:
    case QVariant::SizePolicy:
    default:
        return new VoidVariableImpl();
    }
}

QPair<QString, QString> SplitComment(const QString& comment)
{
    const int separatorIndex = comment.indexOf('/');
    if (separatorIndex < 0)
        return qMakePair(QString(), comment);
    else
        return qMakePair(comment.left(separatorIndex), comment.mid(separatorIndex + 1));
}

ConfigurationVariable::ConfigurationVariable(Configuration& config, const QString& name)
    : config_(config)
    , name_(name)
    , displayName_(SplitComment(config.GetComment(name_)).second)
    , defaultValue_(config.GetDefaultValue(name_))
    , decoration_(config.GetDecoration(name_))
    , impl_(CreateVariable(defaultValue_.type(), decoration_))
{
    impl_->SetValue(config_.GetValue(name_));
}

const QString& ConfigurationVariable::GetDisplayName() const
{
    return displayName_.isEmpty() ? name_ : displayName_;
}

QWidget* ConfigurationVariable::GetWidget()
{
    return impl_->GetWidget();
}

void ConfigurationVariable::Save()
{
    config_.SetValue(name_, impl_->GetValue());
}

void ConfigurationVariable::Reset()
{
    impl_->SetValue(defaultValue_);
}

//////////////////////////////////////////////////////////////////////////
const QString OptionsDialog::GROUP_OTHER = "(Other)";

OptionsDialog::OptionsDialog(Configuration& config)
    : QDialog()
    , config_(config)
    , currentGroup_(nullptr)
{
    setWindowTitle("Options");
    SetupVariables();
    SetupLayout();
}

void OptionsDialog::Save()
{
    for (VariableGroup& group : variables_)
        for (ConfigurationVariable* variable : group)
            variable->Save();
    config_.Save();
}

void OptionsDialog::Reset()
{
    for (VariableGroup& group : variables_)
        for (ConfigurationVariable* variable : group)
            variable->Reset();
}

void OptionsDialog::ResetGroup(const QString& groupName)
{
    if (variables_.contains(groupName))
    {
        for (ConfigurationVariable* variable : variables_[groupName])
            variable->Reset();
    }
}

void OptionsDialog::HandleListRowChanged(int row)
{
    for (QWidget* group : groups_)
        group->setVisible(false);

    currentGroup_ = (row >= 0 && row < groups_.size()) ? groups_[row] : nullptr;

    if (currentGroup_)
        currentGroup_->setVisible(true);
}

void OptionsDialog::HandleOk()
{
    Save();
    close();
}

void OptionsDialog::HandleApply()
{
    Save();
}

void OptionsDialog::HandleCancel()
{
    close();
}

void OptionsDialog::HandleResetThese()
{
    if (currentGroup_)
        ResetGroup(currentGroup_->objectName());
}

void OptionsDialog::HandleResetAll()
{
    Reset();
}

void OptionsDialog::SetupVariables()
{
    const Configuration::VariableMap& variables = config_.GetVariables();
    for (Configuration::VariableMap::ConstIterator iter = variables.begin(); iter != variables.end(); ++iter)
    {
        const QString& name = iter.key();
        const QString comment = config_.GetComment(name);
        const QString group = SplitComment(comment).first;

        ConfigurationVariable* variable = new ConfigurationVariable(config_, name);
        variable->setParent(this);
        variables_[group.isEmpty() ? GROUP_OTHER : group].push_back(variable);
    }

    for (VariableGroup& group : variables_)
    {
        qSort(group.begin(), group.end(),
            [](ConfigurationVariable* lhs, ConfigurationVariable* rhs)
        {
            return lhs->GetDisplayName().compare(rhs->GetDisplayName(), Qt::CaseInsensitive) < 0;
        });
    }
}

void OptionsDialog::SetupLayout()
{
    // Add buttons
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();

    QPushButton* buttonOk = new QPushButton("OK");
    QPushButton* buttonApply = new QPushButton("Apply");
    QPushButton* buttonCancel = new QPushButton("Cancel");
    QPushButton* buttonResetThese = new QPushButton("Reset These");
    QPushButton* buttonResetAll = new QPushButton("Reset All");

    buttonOk->setFocusPolicy(Qt::TabFocus);
    buttonApply->setFocusPolicy(Qt::TabFocus);
    buttonCancel->setFocusPolicy(Qt::TabFocus);
    buttonResetThese->setFocusPolicy(Qt::TabFocus);
    buttonResetAll->setFocusPolicy(Qt::TabFocus);

    connect(buttonOk, SIGNAL(clicked()), this, SLOT(HandleOk()));
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(HandleApply()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(HandleCancel()));
    connect(buttonResetThese, SIGNAL(clicked()), this, SLOT(HandleResetThese()));
    connect(buttonResetAll, SIGNAL(clicked()), this, SLOT(HandleResetAll()));

    buttonsLayout->addWidget(buttonResetAll);
    buttonsLayout->addWidget(buttonResetThese);
    buttonsLayout->addWidget(buttonCancel);
    buttonsLayout->addWidget(buttonApply);
    buttonsLayout->addWidget(buttonOk);

    // Add list
    QListWidget* groupsList = new QListWidget;
    QStringList groups = variables_.keys();
    groups.sort(Qt::CaseInsensitive);
    groupsList->addItems(groups);
    connect(groupsList, SIGNAL(currentRowChanged(int)), this, SLOT(HandleListRowChanged(int)));

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(groupsList);

    // Add variables
    for (const QString& group : groups)
    {
        QScrollArea* variablesGroupArea = new QScrollArea;
        variablesGroupArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        variablesGroupArea->setVisible(false);
        variablesGroupArea->setObjectName(group);
        mainLayout->addWidget(variablesGroupArea);
        mainLayout->setStretchFactor(variablesGroupArea, 1);
        groups_.push_back(variablesGroupArea);

        QWidget* variablesGroup = new QWidget;
        QFormLayout* variablesLayout = new QFormLayout;
        variablesGroup->setLayout(variablesLayout);

        for (ConfigurationVariable* variable : variables_[group])
            variablesLayout->addRow(variable->GetDisplayName(), variable->GetWidget());

        variablesGroupArea->setWidget(variablesGroup);
    }

    // Select page
    if (groups_.size() > 0)
    {
        const int currentIndex = qMin(groups_.size() - 1, 1);
        groupsList->setCurrentRow(currentIndex);
        HandleListRowChanged(currentIndex);
    }

    // Setup dialog layout
    QVBoxLayout* dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(mainLayout);
    dialogLayout->addLayout(buttonsLayout);
    setLayout(dialogLayout);
}

}
