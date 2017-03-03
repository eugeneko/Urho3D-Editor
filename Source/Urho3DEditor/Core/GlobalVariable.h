#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class QSettings;

namespace Urho3DEditor
{

/// Global variable that is stored between program runs.
class GlobalVariable : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    GlobalVariable(const QString& name, const QVariant& defaultValue,
        const QString& section = QString(), const QString& displayText = QString(), const QVariant& decoration = QVariant());
    /// Set context.
    void SetContext(QSettings& settings);

    /// Save value.
    void Save();
    /// Set value.
    void SetValue(const QVariant& value, bool save = true);
    /// Reset to default.
    void ResetToDefault(bool save = true);

    /// Get name.
    const QString& GetName() const { return name_; }
    /// Get section.
    const QString& GetSection() const { return section_; }
    /// Get display text.
    const QString GetDisplayText() const { return tr(qPrintable(displayText_)); }
    /// Get decoration info.
    const QVariant& GetDecorationInfo() const { return decoration_; }
    /// Get default value.
    const QVariant& GetDefaultValue() const { return defaultValue_; }
    /// Get value.
    const QVariant& GetValue() const { return value_; }
    
private:
    /// Context.
    QSettings* settings_ = nullptr;
    /// Variable name.
    QString name_;
    /// Default value.
    QVariant defaultValue_;
    /// Section.
    QString section_;
    /// Display text.
    QString displayText_;
    /// Custom data used for decoration.
    QVariant decoration_;

    /// Current value.
    QVariant value_;
};

/// Template wrapper around global variable.
template <class TPublic, class TInternal = TPublic>
class GlobalVariableT : public GlobalVariable
{
    using GlobalVariable::GetValue;
    using GlobalVariable::SetValue;
public:
    /// Construct.
    GlobalVariableT(const QString& name, const TPublic& defaultValue,
        const QString& section = QString(), const QString& displayText = QString(), const QVariant& decoration = QVariant())
        : GlobalVariable(name, static_cast<TInternal>(defaultValue), section, displayText, decoration)
    {
    }
    /// Get value.
    TPublic GetValue() const { return static_cast<TPublic>(GlobalVariable::GetValue().value<TInternal>()); }
    /// Set value.
    void SetValue(const TPublic& value) { GlobalVariable::SetValue(static_cast<TInternal>(value)); }
};

}
