#pragma once

#include "solver/Load.h"

#include <QDialog>
#include <QString>

#include <optional>
#include <vector>

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;

struct LoadBoundaryConditionOption
{
    QString id;
    QString displayName;
};

struct LoadDialogOptions
{
    std::vector<LoadBoundaryConditionOption> boundaryConditions;
    QString defaultBoundaryConditionId;
};

class LoadDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit LoadDialog(
        LoadDialogOptions options = {},
        QWidget *parent = nullptr
    );

    Load load() const;
    void setLoad(const Load &ld);

    static std::optional<Load> createLoad(
        QWidget *parent,
        LoadDialogOptions options = {}
    );
    static std::optional<Load> editLoad(
        QWidget *parent,
        const Load &existing,
        LoadDialogOptions options = {}
    );

private:
    void setupUi();
    void updateUnitItems();
    void setComboCurrentData(QComboBox *combo, const QString &value);
    void setComboCurrentText(QComboBox *combo, const QString &text);
    QString selectedBoundaryConditionId() const;

    LoadDialogOptions m_options;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QComboBox *m_boundaryConditionIdCombo = nullptr;
    QLineEdit *m_fieldNameEdit = nullptr;
    QDoubleSpinBox *m_valueSpin = nullptr;
    QComboBox *m_unitCombo = nullptr;
};
