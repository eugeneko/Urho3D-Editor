#pragma once

#include <QDialog>
#include <QVariant>

class QVBoxLayout;

namespace Urho3DEditor
{

class Configuration;

class ConfigurationVariableImpl;

/// Variable Widget.
class ConfigurationVariable : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    ConfigurationVariable(Configuration& config, const QString& name,
        const QVariant& defaultValue, const QString& displayText, const QVariant& decorationInfo);
    /// Get display text.
    const QString& GetDisplayText() const;
    /// Get widget.
    QWidget* GetWidget();
    /// Save variable value.
    void Save();
    /// Reset variable value to default.
    void Reset();

private:
    /// Config.
    Configuration& config_;
    /// Name.
    const QString name_;
    /// Display name.
    const QString displayText_;
    /// Default value.
    const QVariant defaultValue_;
    /// Decoration info.
    const QVariant decorationInfo_;
    /// Implementation details.
    QScopedPointer<ConfigurationVariableImpl> impl_;

};

/// Options Dialog Widget.
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    /// Section that contains variables.
    using Section = QVector<ConfigurationVariable*>;

public:
    /// Construct.
    OptionsDialog(Configuration& config);
    /// Save variables.
    void Save();
    /// Reset variables.
    void Reset();
    /// Reset variables from section.
    void ResetSection(const QString& sectionName);

private slots:
    /// Handle list item selected.
    void HandleListRowChanged(int row);
    /// Handle 'OK' button clicked.
    void HandleOk();
    /// Handle 'Apply' button clicked.
    void HandleApply();
    /// Handle 'Cancel' button clicked.
    void HandleCancel();
    /// Handle 'Reset Page' button clicked.
    void HandleResetThese();
    /// Handle 'Reset All' button clicked.
    void HandleResetAll();

private:
    /// Setup variables.
    void SetupVariables();
    /// Setup dialog layout.
    void SetupLayout();

private:
    /// Config.
    Configuration& config_;
    /// Dialog layout.
    QScopedPointer<QVBoxLayout> dialogLayout_;
    /// Variables.
    QHash<QString, Section> variables_;
    /// Section widgets.
    QVector<QWidget*> sections_;
    /// Current section.
    QWidget* currentSection_;

};

}
