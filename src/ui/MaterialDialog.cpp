#include "MaterialDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

namespace
{
double materialPropertyValue(const Material &material, const QString &propertyName, double fallback = 0.0)
{
    for (const MaterialProperty &property : material.extraProperties) {
        if (property.name.compare(propertyName, Qt::CaseInsensitive) == 0) {
            return property.value;
        }
    }
    return fallback;
}
}

MaterialDialog::MaterialDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Create Material");
    setupUi();
}

void MaterialDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("e.g. Water, Air, Steel");
    form->addRow("Name:", m_nameEdit);

    m_domainCombo = new QComboBox(this);
    m_domainCombo->addItem("Fluid", static_cast<int>(MaterialDomain::Fluid));
    m_domainCombo->addItem("Solid", static_cast<int>(MaterialDomain::Solid));
    form->addRow("Domain:", m_domainCombo);
    connect(m_domainCombo, &QComboBox::currentIndexChanged, this, &MaterialDialog::onDomainChanged);

    m_viscosityCombo = new QComboBox(this);
    m_viscosityCombo->addItem("Newtonian", static_cast<int>(ViscosityModel::Newtonian));
    form->addRow("Viscosity Model:", m_viscosityCombo);

    // --- Density ---
    m_hasDensityCheck = new QCheckBox("Enable Density", this);
    form->addRow(m_hasDensityCheck);
    m_densitySpin = new QDoubleSpinBox(this);
    m_densitySpin->setRange(0.0, 1e9);
    m_densitySpin->setDecimals(4);
    m_densitySpin->setSuffix(" kg/m^3");
    m_densitySpin->setEnabled(false);
    form->addRow("Density:", m_densitySpin);

    connect(m_hasDensityCheck, &QCheckBox::toggled, m_densitySpin, &QDoubleSpinBox::setEnabled);

    // --- Dynamic Viscosity ---
    m_hasDynamicViscosityCheck = new QCheckBox("Enable Dynamic Viscosity", this);
    form->addRow(m_hasDynamicViscosityCheck);
    m_dynamicViscositySpin = new QDoubleSpinBox(this);
    m_dynamicViscositySpin->setRange(0.0, 1e9);
    m_dynamicViscositySpin->setDecimals(6);
    m_dynamicViscositySpin->setSuffix(" Pa*s");
    m_dynamicViscositySpin->setEnabled(false);
    form->addRow("Dynamic Viscosity:", m_dynamicViscositySpin);

    connect(m_hasDynamicViscosityCheck, &QCheckBox::toggled, m_dynamicViscositySpin, &QDoubleSpinBox::setEnabled);

    // --- Kinematic Viscosity ---
    m_hasKinematicViscosityCheck = new QCheckBox("Enable Kinematic Viscosity", this);
    form->addRow(m_hasKinematicViscosityCheck);
    m_kinematicViscositySpin = new QDoubleSpinBox(this);
    m_kinematicViscositySpin->setRange(0.0, 1e9);
    m_kinematicViscositySpin->setDecimals(6);
    m_kinematicViscositySpin->setSuffix(" m^2/s");
    m_kinematicViscositySpin->setEnabled(false);
    form->addRow("Kinematic Viscosity:", m_kinematicViscositySpin);

    connect(m_hasKinematicViscosityCheck, &QCheckBox::toggled, m_kinematicViscositySpin, &QDoubleSpinBox::setEnabled);

    m_youngModulusSpin = new QDoubleSpinBox(this);
    m_youngModulusSpin->setRange(0.0, 1e15);
    m_youngModulusSpin->setDecimals(3);
    m_youngModulusSpin->setSingleStep(1e9);
    m_youngModulusSpin->setValue(2.1e11);
    m_youngModulusSpin->setSuffix(" Pa");
    form->addRow("Young Modulus:", m_youngModulusSpin);

    m_poissonRatioSpin = new QDoubleSpinBox(this);
    m_poissonRatioSpin->setRange(0.0, 0.499999);
    m_poissonRatioSpin->setDecimals(6);
    m_poissonRatioSpin->setSingleStep(0.01);
    m_poissonRatioSpin->setValue(0.3);
    form->addRow("Poisson Ratio:", m_poissonRatioSpin);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Material name cannot be empty.");
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    onDomainChanged(m_domainCombo->currentIndex());
}

void MaterialDialog::onDomainChanged(int index)
{
    Q_UNUSED(index);
    const bool solid = static_cast<MaterialDomain>(m_domainCombo->currentData().toInt()) == MaterialDomain::Solid;
    m_viscosityCombo->setEnabled(!solid);
    m_hasDynamicViscosityCheck->setEnabled(!solid);
    m_dynamicViscositySpin->setEnabled(!solid && m_hasDynamicViscosityCheck->isChecked());
    m_hasKinematicViscosityCheck->setEnabled(!solid);
    m_kinematicViscositySpin->setEnabled(!solid && m_hasKinematicViscosityCheck->isChecked());
    m_youngModulusSpin->setEnabled(solid);
    m_poissonRatioSpin->setEnabled(solid);
}

Material MaterialDialog::material() const
{
    Material mat;
    mat.id = m_nameEdit->text().trimmed().toLower().replace(' ', '_');
    mat.name = m_nameEdit->text().trimmed();
    mat.domain = static_cast<MaterialDomain>(m_domainCombo->currentData().toInt());
    mat.viscosityModel = static_cast<ViscosityModel>(m_viscosityCombo->currentData().toInt());

    mat.hasDensity = m_hasDensityCheck->isChecked();
    mat.density = m_densitySpin->value();

    mat.hasDynamicViscosity = m_hasDynamicViscosityCheck->isChecked();
    mat.dynamicViscosity = m_dynamicViscositySpin->value();

    mat.hasKinematicViscosity = m_hasKinematicViscosityCheck->isChecked();
    mat.kinematicViscosity = m_kinematicViscositySpin->value();

    if (mat.domain == MaterialDomain::Solid) {
        mat.extraProperties.push_back({"youngModulus", m_youngModulusSpin->value(), "Pa"});
        mat.extraProperties.push_back({"poissonRatio", m_poissonRatioSpin->value(), ""});
    }

    return mat;
}

void MaterialDialog::setMaterial(const Material &mat)
{
    m_nameEdit->setText(mat.name);
    for (int i = 0; i < m_domainCombo->count(); ++i) {
        if (m_domainCombo->itemData(i).toInt() == static_cast<int>(mat.domain)) {
            m_domainCombo->setCurrentIndex(i);
            break;
        }
    }
    for (int i = 0; i < m_viscosityCombo->count(); ++i) {
        if (m_viscosityCombo->itemData(i).toInt() == static_cast<int>(mat.viscosityModel)) {
            m_viscosityCombo->setCurrentIndex(i);
            break;
        }
    }

    m_hasDensityCheck->setChecked(mat.hasDensity);
    m_densitySpin->setValue(mat.density);

    m_hasDynamicViscosityCheck->setChecked(mat.hasDynamicViscosity);
    m_dynamicViscositySpin->setValue(mat.dynamicViscosity);

    m_hasKinematicViscosityCheck->setChecked(mat.hasKinematicViscosity);
    m_kinematicViscositySpin->setValue(mat.kinematicViscosity);

    m_youngModulusSpin->setValue(materialPropertyValue(mat, "youngModulus", 2.1e11));
    m_poissonRatioSpin->setValue(materialPropertyValue(mat, "poissonRatio", 0.3));
    onDomainChanged(m_domainCombo->currentIndex());
}

std::optional<Material> MaterialDialog::createMaterial(QWidget *parent)
{
    MaterialDialog dlg(parent);
    dlg.setWindowTitle("Create Material");
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return std::nullopt;
}

std::optional<Material> MaterialDialog::editMaterial(QWidget *parent, const Material &existing)
{
    MaterialDialog dlg(parent);
    dlg.setWindowTitle("Edit Material");
    dlg.setMaterial(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return std::nullopt;
}
