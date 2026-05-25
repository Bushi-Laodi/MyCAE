#pragma once

#include "solver/Material.h"

#include <QDialog>

#include <optional>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

class MaterialDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialDialog(QWidget *parent = nullptr);

    Material material() const;
    void setMaterial(const Material &mat);

    static std::optional<Material> createMaterial(QWidget *parent);
    static std::optional<Material> editMaterial(QWidget *parent, const Material &existing);

private:
    void setupUi();
    void onDomainChanged(int index);

    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_domainCombo = nullptr;
    QComboBox *m_viscosityCombo = nullptr;

    QCheckBox *m_hasDensityCheck = nullptr;
    QDoubleSpinBox *m_densitySpin = nullptr;

    QCheckBox *m_hasDynamicViscosityCheck = nullptr;
    QDoubleSpinBox *m_dynamicViscositySpin = nullptr;

    QCheckBox *m_hasKinematicViscosityCheck = nullptr;
    QDoubleSpinBox *m_kinematicViscositySpin = nullptr;

    QDoubleSpinBox *m_youngModulusSpin = nullptr;
    QDoubleSpinBox *m_poissonRatioSpin = nullptr;
};
