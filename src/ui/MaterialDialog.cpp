#include "MaterialDialog.h"

#include "units/UnitConverter.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

double materialPropertyValue(const Material &material, const QString &propertyName, double fallback = 0.0)
{
    for (const MaterialProperty &property : material.extraProperties) {
        if (property.name.compare(propertyName, Qt::CaseInsensitive) == 0) {
            return property.value;
        }
    }
    return fallback;
}

QString materialPropertyUnit(const Material &material, const QString &propertyName, const QString &fallback = {})
{
    for (const MaterialProperty &property : material.extraProperties) {
        if (property.name.compare(propertyName, Qt::CaseInsensitive) == 0) {
            return property.unit.trimmed().isEmpty() ? fallback : property.unit;
        }
    }
    return fallback;
}
}

MaterialDialog::MaterialDialog(MaterialDialogOptions options, QWidget *parent)
    : QDialog(parent)
    , m_options(std::move(options))
{
    setWindowTitle(zh(u8"创建材料"));
    setupUi();
}

void MaterialDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(zh(u8"例如：水、空气、钢"));
    form->addRow(zh(u8"名称:"), m_nameEdit);

    m_domainCombo = new QComboBox(this);
    m_domainCombo->addItem(zh(u8"流体"), static_cast<int>(MaterialDomain::Fluid));
    m_domainCombo->addItem(zh(u8"固体"), static_cast<int>(MaterialDomain::Solid));
    if (m_options.fixedDomain) {
        for (int i = 0; i < m_domainCombo->count(); ++i) {
            if (m_domainCombo->itemData(i).toInt() == static_cast<int>(*m_options.fixedDomain)) {
                m_domainCombo->setCurrentIndex(i);
                break;
            }
        }
        m_domainCombo->setEnabled(false);
    }
    form->addRow(zh(u8"物理域:"), m_domainCombo);
    connect(m_domainCombo, &QComboBox::currentIndexChanged, this, &MaterialDialog::onDomainChanged);

    m_viscosityCombo = new QComboBox(this);
    m_viscosityCombo->addItem(zh(u8"牛顿流体"), static_cast<int>(ViscosityModel::Newtonian));
    form->addRow(zh(u8"黏度模型:"), m_viscosityCombo);

    // --- Density ---
    m_hasDensityCheck = new QCheckBox(zh(u8"启用密度"), this);
    form->addRow(m_hasDensityCheck);
    m_densitySpin = new QDoubleSpinBox(this);
    m_densitySpin->setRange(0.0, 1e9);
    m_densitySpin->setDecimals(4);
    m_densitySpin->setSuffix(" kg/m^3");
    m_densitySpin->setEnabled(false);
    form->addRow(zh(u8"密度:"), m_densitySpin);

    connect(m_hasDensityCheck, &QCheckBox::toggled, m_densitySpin, &QDoubleSpinBox::setEnabled);

    // --- Dynamic Viscosity ---
    m_hasDynamicViscosityCheck = new QCheckBox(zh(u8"启用动力黏度"), this);
    form->addRow(m_hasDynamicViscosityCheck);
    m_dynamicViscositySpin = new QDoubleSpinBox(this);
    m_dynamicViscositySpin->setRange(0.0, 1e9);
    m_dynamicViscositySpin->setDecimals(6);
    m_dynamicViscositySpin->setSuffix(" Pa*s");
    m_dynamicViscositySpin->setEnabled(false);
    form->addRow(zh(u8"动力黏度:"), m_dynamicViscositySpin);

    connect(m_hasDynamicViscosityCheck, &QCheckBox::toggled, m_dynamicViscositySpin, &QDoubleSpinBox::setEnabled);

    // --- Kinematic Viscosity ---
    m_hasKinematicViscosityCheck = new QCheckBox(zh(u8"启用运动黏度"), this);
    form->addRow(m_hasKinematicViscosityCheck);
    m_kinematicViscositySpin = new QDoubleSpinBox(this);
    m_kinematicViscositySpin->setRange(0.0, 1e9);
    m_kinematicViscositySpin->setDecimals(6);
    m_kinematicViscositySpin->setSuffix(" m^2/s");
    m_kinematicViscositySpin->setEnabled(false);
    form->addRow(zh(u8"运动黏度:"), m_kinematicViscositySpin);

    connect(m_hasKinematicViscosityCheck, &QCheckBox::toggled, m_kinematicViscositySpin, &QDoubleSpinBox::setEnabled);

    m_youngModulusSpin = new QDoubleSpinBox(this);
    m_youngModulusSpin->setRange(0.0, 1e15);
    m_youngModulusSpin->setDecimals(3);
    m_youngModulusSpin->setSingleStep(1.0);
    m_youngModulusSpin->setValue(210.0);
    form->addRow(zh(u8"杨氏模量:"), m_youngModulusSpin);

    m_youngModulusUnitCombo = new QComboBox(this);
    m_youngModulusUnitCombo->addItems({"Pa", "MPa", "GPa"});
    m_youngModulusUnitCombo->setCurrentText("GPa");
    form->addRow(zh(u8"杨氏模量单位:"), m_youngModulusUnitCombo);

    m_poissonRatioSpin = new QDoubleSpinBox(this);
    m_poissonRatioSpin->setRange(0.0, 0.499999);
    m_poissonRatioSpin->setDecimals(6);
    m_poissonRatioSpin->setSingleStep(0.01);
    m_poissonRatioSpin->setValue(0.3);
    form->addRow(zh(u8"泊松比:"), m_poissonRatioSpin);

    m_thermalExpansionSpin = new QDoubleSpinBox(this);
    m_thermalExpansionSpin->setRange(0.0, 1.0);
    m_thermalExpansionSpin->setDecimals(12);
    m_thermalExpansionSpin->setSingleStep(1.0e-6);
    m_thermalExpansionSpin->setValue(0.0);
    m_thermalExpansionSpin->setSuffix(" 1/K");
    form->addRow(zh(u8"热膨胀系数 alpha:"), m_thermalExpansionSpin);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"材料名称不能为空。"));
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
    m_youngModulusUnitCombo->setEnabled(solid);
    m_poissonRatioSpin->setEnabled(solid);
    m_thermalExpansionSpin->setEnabled(solid);
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
        mat.extraProperties.push_back({"youngModulus", m_youngModulusSpin->value(), m_youngModulusUnitCombo->currentText()});
        mat.extraProperties.push_back({"poissonRatio", m_poissonRatioSpin->value(), ""});
        if (m_thermalExpansionSpin->value() > 0.0) {
            mat.extraProperties.push_back({"thermalExpansion", m_thermalExpansionSpin->value(), "1/K"});
        }
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

    const QString youngUnit = materialPropertyUnit(mat, "youngModulus", "GPa");
    const int unitIndex = m_youngModulusUnitCombo->findText(youngUnit, Qt::MatchFixedString);
    if (unitIndex >= 0) {
        m_youngModulusUnitCombo->setCurrentIndex(unitIndex);
    }
    m_youngModulusSpin->setValue(materialPropertyValue(mat, "youngModulus", youngUnit == "GPa" ? 210.0 : 2.1e11));
    m_poissonRatioSpin->setValue(materialPropertyValue(mat, "poissonRatio", 0.3));
    m_thermalExpansionSpin->setValue(materialPropertyValue(mat, "thermalExpansion", 0.0));
    onDomainChanged(m_domainCombo->currentIndex());
}

std::optional<Material> MaterialDialog::createMaterial(QWidget *parent, MaterialDialogOptions options)
{
    MaterialDialog dlg(std::move(options), parent);
    dlg.setWindowTitle(zh(u8"创建材料"));
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return std::nullopt;
}

std::optional<Material> MaterialDialog::editMaterial(QWidget *parent, const Material &existing)
{
    MaterialDialog dlg({}, parent);
    dlg.setWindowTitle(zh(u8"编辑材料"));
    dlg.setMaterial(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.material();
    }
    return std::nullopt;
}
