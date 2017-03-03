#include "GlobalVariable.h"
#include <QSettings>

namespace Urho3DEditor
{

GlobalVariable::GlobalVariable(const QString& name, const QVariant& defaultValue,
    const QString& section /*= QString()*/, const QString& displayText /*= QString()*/, const QVariant& decoration /*= QVariant()*/)
    : name_(name)
    , defaultValue_(defaultValue)
    , section_(section)
    , displayText_(displayText)
    , decoration_(decoration)
    , value_(defaultValue)
{

}

void GlobalVariable::SetContext(QSettings& settings)
{
    settings_ = &settings;
    value_ = settings.value(name_, defaultValue_);
}

void GlobalVariable::Save()
{
    if (settings_)
        settings_->setValue(name_, value_);
}

void GlobalVariable::SetValue(const QVariant& value, bool save /*= true*/)
{
    value_ = value;
    if (save)
        Save();
}

void GlobalVariable::ResetToDefault(bool save /*= true*/)
{
    SetValue(defaultValue_);
    if (save)
        Save();
}

}
