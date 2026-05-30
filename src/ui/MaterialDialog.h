#pragma once

#include "solver/Material.h"

#include <QDialog>

#include <optional>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

struct MaterialDialogOptions
{
    std::optional<MaterialDomain> fixedDomain;
};

class MaterialDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialDialog(MaterialDialogOptions options = {}, QWidget *parent = nullptr);

    Material material() const;
    void setMaterial(const Material &mat);

    static std::optional<Material> createMaterial(QWidget *parent, MaterialDialogOptions options = {});
    static std::optional<Material> editMaterial(QWidget *parent, const Material &existing);

private:
    void setupUi();
    void onDomainChanged(int index);

    MaterialDialogOptions m_options;
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
    QComboBox *m_youngModulusUnitCombo = nullptr;
    QDoubleSpinBox *m_poissonRatioSpin = nullptr;
};
