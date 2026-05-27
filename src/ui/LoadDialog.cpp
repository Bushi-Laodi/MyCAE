#include "LoadDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

#include <utility>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString defaultFieldName(LoadType type)
{
    switch (type) {
    case LoadType::Velocity:
        return "U";
    case LoadType::Pressure:
        return "pressure";
    case LoadType::BodyForce:
        return "bodyForce";
    case LoadType::Unknown:
        return {};
    }
    return {};
}

bool isDefaultFieldName(const QString &fieldName)
{
    const QString normalized = fieldName.trimmed();
    return normalized.isEmpty()
        || normalized == defaultFieldName(LoadType::Velocity)
        || normalized == defaultFieldName(LoadType::Pressure)
        || normalized == defaultFieldName(LoadType::BodyForce);
}

QStringList unitOptions(LoadType type)
{
    switch (type) {
    case LoadType::Velocity:
        return {"m/s", "mm/s"};
    case LoadType::Pressure:
        return {"Pa", "kPa", "MPa", "N/mm^2"};
    case LoadType::BodyForce:
        return {"N", "N/m^3", "N/mm^3", "m/s^2"};
    case LoadType::Unknown:
        return {};
    }
    return {};
}

LoadType selectedLoadType(const QComboBox *combo)
{
    return static_cast<LoadType>(combo->currentData().toInt());
}
}

LoadDialog::LoadDialog(
    LoadDialogOptions options,
    QWidget *parent
)
    : QDialog(parent)
    , m_options(std::move(options))
{
    setWindowTitle(zh(u8"创建载荷"));
    setupUi();
}

void LoadDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(zh(u8"例如：入口速度、端面压力"));
    form->addRow(zh(u8"名称:"), m_nameEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(zh(u8"速度"), static_cast<int>(LoadType::Velocity));
    m_typeCombo->addItem(zh(u8"压力"), static_cast<int>(LoadType::Pressure));
    m_typeCombo->addItem(zh(u8"体力"), static_cast<int>(LoadType::BodyForce));
    form->addRow(zh(u8"类型:"), m_typeCombo);

    m_boundaryConditionIdCombo = new QComboBox(this);
    for (const LoadBoundaryConditionOption &option : m_options.boundaryConditions) {
        const QString id = option.id.trimmed();
        if (id.isEmpty()) {
            continue;
        }
        const QString label = option.displayName.trimmed().isEmpty()
            ? id
            : option.displayName.trimmed();
        m_boundaryConditionIdCombo->addItem(label, id);
    }
    m_boundaryConditionIdCombo->setEditable(false);
    if (!m_options.defaultBoundaryConditionId.trimmed().isEmpty()) {
        setComboCurrentData(m_boundaryConditionIdCombo, m_options.defaultBoundaryConditionId);
    }
    form->addRow(zh(u8"边界条件:"), m_boundaryConditionIdCombo);

    m_fieldNameEdit = new QLineEdit(this);
    m_fieldNameEdit->setPlaceholderText(zh(u8"例如：U（速度），p（压力）"));
    form->addRow(zh(u8"场名称:"), m_fieldNameEdit);

    m_valueSpin = new QDoubleSpinBox(this);
    m_valueSpin->setRange(-1e9, 1e9);
    m_valueSpin->setDecimals(4);
    form->addRow(zh(u8"数值:"), m_valueSpin);

    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(false);
    form->addRow(zh(u8"单位:"), m_unitCombo);

    connect(m_typeCombo, &QComboBox::currentTextChanged, this, [this]() {
        const QString currentDefault = defaultFieldName(selectedLoadType(m_typeCombo));
        if (isDefaultFieldName(m_fieldNameEdit->text())) {
            m_fieldNameEdit->setText(currentDefault);
        }
        updateUnitItems();
    });
    m_fieldNameEdit->setText(defaultFieldName(selectedLoadType(m_typeCombo)));
    updateUnitItems();

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"载荷名称不能为空。"));
            return;
        }
        if (selectedBoundaryConditionId().isEmpty()) {
            QMessageBox::warning(
                this,
                zh(u8"校验"),
                zh(u8"创建载荷前，请先创建并选择一个边界条件。")
            );
            return;
        }
        if (m_unitCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"请选择载荷单位。"));
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

Load LoadDialog::load() const
{
    Load ld;
    ld.id = m_nameEdit->text().trimmed().toLower().replace(' ', '_');
    ld.name = m_nameEdit->text().trimmed();
    ld.type = static_cast<LoadType>(m_typeCombo->currentData().toInt());
    ld.boundaryConditionId = selectedBoundaryConditionId();
    ld.fieldName = m_fieldNameEdit->text().trimmed();
    ld.value.kind = LoadValueKind::Scalar;
    ld.value.x = m_valueSpin->value();
    ld.value.unit = m_unitCombo->currentText().trimmed();
    ld.enabled = true;
    return ld;
}

void LoadDialog::setLoad(const Load &ld)
{
    m_nameEdit->setText(ld.name);
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toInt() == static_cast<int>(ld.type)) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }
    setComboCurrentData(m_boundaryConditionIdCombo, ld.boundaryConditionId);
    m_fieldNameEdit->setText(
        ld.fieldName.trimmed().isEmpty()
            ? defaultFieldName(ld.type)
            : ld.fieldName
    );
    m_valueSpin->setValue(ld.value.x);
    updateUnitItems();
    setComboCurrentText(m_unitCombo, ld.value.unit);
}

void LoadDialog::updateUnitItems()
{
    const QString currentUnit = m_unitCombo->currentText().trimmed();
    const QStringList options = unitOptions(selectedLoadType(m_typeCombo));

    m_unitCombo->blockSignals(true);
    m_unitCombo->clear();
    m_unitCombo->addItems(options);
    const int existingIndex = m_unitCombo->findText(currentUnit);
    if (existingIndex >= 0) {
        m_unitCombo->setCurrentIndex(existingIndex);
    }
    m_unitCombo->blockSignals(false);
}

void LoadDialog::setComboCurrentData(QComboBox *combo, const QString &value)
{
    const QString trimmedValue = value.trimmed();
    if (trimmedValue.isEmpty()) {
        return;
    }

    const int existingIndex = combo->findData(trimmedValue);
    if (existingIndex >= 0) {
        combo->setCurrentIndex(existingIndex);
        return;
    }

    combo->addItem(trimmedValue, trimmedValue);
    combo->setCurrentIndex(combo->count() - 1);
}

void LoadDialog::setComboCurrentText(QComboBox *combo, const QString &text)
{
    const QString trimmedText = text.trimmed();
    if (trimmedText.isEmpty()) {
        return;
    }

    const int existingIndex = combo->findText(trimmedText);
    if (existingIndex >= 0) {
        combo->setCurrentIndex(existingIndex);
        return;
    }

    combo->addItem(trimmedText);
    combo->setCurrentIndex(combo->count() - 1);
}

QString LoadDialog::selectedBoundaryConditionId() const
{
    return m_boundaryConditionIdCombo->currentData().toString().trimmed();
}

std::optional<Load> LoadDialog::createLoad(
    QWidget *parent,
    LoadDialogOptions options
)
{
    LoadDialog dlg(std::move(options), parent);
    dlg.setWindowTitle(zh(u8"创建载荷"));
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.load();
    }
    return std::nullopt;
}

std::optional<Load> LoadDialog::editLoad(
    QWidget *parent,
    const Load &existing,
    LoadDialogOptions options
)
{
    LoadDialog dlg(std::move(options), parent);
    dlg.setWindowTitle(zh(u8"编辑载荷"));
    dlg.setLoad(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.load();
    }
    return std::nullopt;
}
