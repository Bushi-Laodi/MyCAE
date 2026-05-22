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
}

void MaterialDialog::onDomainChanged(int index)
{
    Q_UNUSED(index);
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

    return mat;
}

void MaterialDialog::setMaterial(const Material &mat)
{
    m_nameEdit->setText(mat.name);
    m_domainCombo->setCurrentIndex(static_cast<int>(mat.domain));
    m_viscosityCombo->setCurrentIndex(static_cast<int>(mat.viscosityModel));

    m_hasDensityCheck->setChecked(mat.hasDensity);
    m_densitySpin->setValue(mat.density);

    m_hasDynamicViscosityCheck->setChecked(mat.hasDynamicViscosity);
    m_dynamicViscositySpin->setValue(mat.dynamicViscosity);

    m_hasKinematicViscosityCheck->setChecked(mat.hasKinematicViscosity);
    m_kinematicViscositySpin->setValue(mat.kinematicViscosity);
}

Material MaterialDialog::createMaterial(QWidget *parent)
{
    MaterialDialog dlg(parent);
    dlg.setWindowTitle("Create Material");
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return {};
}

Material MaterialDialog::editMaterial(QWidget *parent, const Material &existing)
{
    MaterialDialog dlg(parent);
    dlg.setWindowTitle("Edit Material");
    dlg.setMaterial(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return {};
}
