#pragma once

#include "solver/Load.h"

#include <QDialog>

#include <optional>

class QLineEdit;
class QComboBox;
class QDoubleSpinBox;

class LoadDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit LoadDialog(QWidget *parent = nullptr);

    Load load() const;
    void setLoad(const Load &ld);

    static std::optional<Load> createLoad(QWidget *parent);
    static std::optional<Load> editLoad(QWidget *parent, const Load &existing);

private:
    void setupUi();

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QLineEdit *m_boundaryConditionIdEdit = nullptr;
    QLineEdit *m_fieldNameEdit = nullptr;
    QDoubleSpinBox *m_valueSpin = nullptr;
    QLineEdit *m_unitEdit = nullptr;
};
