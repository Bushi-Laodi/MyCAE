#pragma once

#include "solver/BoundaryCondition.h"

#include <QDialog>

#include <optional>

class QLineEdit;
class QComboBox;

class BoundaryConditionDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BoundaryConditionDialog(QWidget *parent = nullptr);

    BoundaryCondition boundaryCondition() const;
    void setBoundaryCondition(const BoundaryCondition &bc);

    static std::optional<BoundaryCondition> createBoundaryCondition(QWidget *parent);
    static std::optional<BoundaryCondition> editBoundaryCondition(QWidget *parent, const BoundaryCondition &existing);

private:
    void setupUi();

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QLineEdit *m_geometryNameEdit = nullptr;
    QLineEdit *m_faceGroupNameEdit = nullptr;
    QLineEdit *m_materialIdEdit = nullptr;
};
