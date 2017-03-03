#pragma once

#include <QDialog>
#include <QVariant>

class QVBoxLayout;

namespace Urho3DEditor
{

class Core;
class GlobalVariable;

class ConfigurationVariableImpl;

/// Variable Widget.
class GlobalVariableFacade : public QObject
{
    Q_OBJECT

public:
    /// Construct.
    GlobalVariableFacade(GlobalVariable& variable);

    /// Reset value to default.
    void ResetToDefault();
    /// Save new value to variable.
    void Save();

    /// Get display text.
    const QString GlobalVariableFacade::GetDisplayText() const;
    /// Get variable.
    GlobalVariable& GetVariable() { return variable_; }
    /// Get widget.
    QWidget* GetWidget();

private:
    /// Variable.
    GlobalVariable& variable_;
    /// Implementation details.
    QScopedPointer<ConfigurationVariableImpl> impl_;

};

/// Options Dialog Widget.
class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    /// Construct.
    OptionsDialog(Core& core);
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
    /// Core.
    Core& core_;
    /// Dialog layout.
    QScopedPointer<QVBoxLayout> dialogLayout_;
    /// Variables.
    QHash<QString, QVector<GlobalVariableFacade*>> variables_;
    /// Section widgets.
    QVector<QWidget*> sections_;
    /// Current section.
    QWidget* currentSection_;

};

}
