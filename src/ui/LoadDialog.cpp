#include "LoadDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
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
    case LoadType::Force:
        return "force";
    case LoadType::SurfaceForce:
        return "surfaceForce";
    case LoadType::Traction:
        return "traction";
    case LoadType::Pressure:
        return "pressure";
    case LoadType::Gravity:
        return "gravity";
    case LoadType::BodyForce:
        return "bodyForce";
    case LoadType::Temperature:
        return "temperature";
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
        || normalized == defaultFieldName(LoadType::Force)
        || normalized == defaultFieldName(LoadType::SurfaceForce)
        || normalized == defaultFieldName(LoadType::Traction)
        || normalized == defaultFieldName(LoadType::Pressure)
        || normalized == defaultFieldName(LoadType::Gravity)
        || normalized == defaultFieldName(LoadType::BodyForce)
        || normalized == defaultFieldName(LoadType::Temperature);
}

bool isVectorLoadType(LoadType type)
{
    return type == LoadType::Force
        || type == LoadType::SurfaceForce
        || type == LoadType::Traction
        || type == LoadType::Gravity
        || type == LoadType::BodyForce;
}

QStringList unitOptions(LoadType type)
{
    switch (type) {
    case LoadType::Velocity:
        return {"m/s", "mm/s"};
    case LoadType::Force:
        return {"N", "kN"};
    case LoadType::SurfaceForce:
        return {"N", "kN"};
    case LoadType::Traction:
        return {"MPa", "N/mm^2"};
    case LoadType::Pressure:
        return {"Pa", "kPa", "MPa", "N/mm^2"};
    case LoadType::Gravity:
        return {"m/s^2", "mm/s^2"};
    case LoadType::BodyForce:
        return {"N", "N/m^3", "N/mm^3", "m/s^2"};
    case LoadType::Temperature:
        return {"K", "C"};
    case LoadType::Unknown:
        return {};
    }
    return {};
}

LoadType selectedLoadType(const QComboBox *combo)
{
    return static_cast<LoadType>(combo->currentData().toInt());
}

QString loadTypeLabel(LoadType type)
{
    if (type == LoadType::SurfaceForce) {
        return zh(u8"等效节点力（由面选择分配）");
    }
    if (type == LoadType::Traction) {
        return zh(u8"真实面力 / Surface traction");
    }
    if (type == LoadType::Temperature) {
        return zh(u8"温度（暂不开放求解）");
    }
    switch (type) {
    case LoadType::Velocity:
        return zh(u8"速度");
    case LoadType::Force:
        return zh(u8"集中力");
    case LoadType::SurfaceForce:
        return zh(u8"等效节点力（由面选择分配）");
    case LoadType::Traction:
        return zh(u8"真实面力 / Surface traction");
    case LoadType::Pressure:
        return zh(u8"压力载荷");
    case LoadType::Gravity:
        return zh(u8"重力");
    case LoadType::BodyForce:
        return zh(u8"体力");
    case LoadType::Temperature:
        return zh(u8"均匀温度变化 V1（暂不开放求解）");
    case LoadType::Unknown:
        return zh(u8"未知");
    }
    return zh(u8"未知");
}

std::vector<LoadType> defaultLoadTypes()
{
    return {LoadType::Velocity, LoadType::Force, LoadType::SurfaceForce, LoadType::Traction, LoadType::Pressure, LoadType::Gravity};
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
    m_nameEdit->setPlaceholderText(zh(u8"例如：端面压力、Z 向力、重力"));
    form->addRow(zh(u8"名称:"), m_nameEdit);

    m_typeCombo = new QComboBox(this);
    const std::vector<LoadType> loadTypes =
        m_options.allowedTypes.empty() ? defaultLoadTypes() : m_options.allowedTypes;
    for (const LoadType type : loadTypes) {
        m_typeCombo->addItem(loadTypeLabel(type), static_cast<int>(type));
    }
    m_typeCombo->setEnabled(loadTypes.size() > 1);
    form->addRow(zh(u8"类型:"), m_typeCombo);
    auto *surfaceForceHintLabel = new QLabel(
        zh(u8"SurfaceForce 当前会按目标边界节点平均分配为 *CLOAD，不是真正的 CalculiX 表面力。"),
        this
    );
    surfaceForceHintLabel->setWordWrap(true);
    surfaceForceHintLabel->setStyleSheet("color: #9a6700;");
    form->addRow(QString(), surfaceForceHintLabel);
    auto *tractionHintLabel = new QLabel(
        zh(u8"Traction V1 会按向量模长导出为 CalculiX *DLOAD 法向面载荷；不会降级为 *CLOAD，切向 traction 暂不支持。"),
        this
    );
    tractionHintLabel->setWordWrap(true);
    tractionHintLabel->setStyleSheet("color: #9a6700;");
    form->addRow(QString(), tractionHintLabel);

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
    m_fieldNameEdit->setPlaceholderText(zh(u8"例如：pressure、force、gravity"));
    form->addRow(zh(u8"场名称:"), m_fieldNameEdit);

    m_valueSpin = new QDoubleSpinBox(this);
    m_yValueSpin = new QDoubleSpinBox(this);
    m_zValueSpin = new QDoubleSpinBox(this);
    for (QDoubleSpinBox *spin : {m_valueSpin, m_yValueSpin, m_zValueSpin}) {
        spin->setRange(-1e12, 1e12);
        spin->setDecimals(6);
    }
    form->addRow(zh(u8"X / 数值:"), m_valueSpin);
    form->addRow("Y:", m_yValueSpin);
    form->addRow("Z:", m_zValueSpin);
    if (QWidget *label = form->labelForField(m_yValueSpin)) {
        label->setObjectName("loadYLabel");
    }
    if (QWidget *label = form->labelForField(m_zValueSpin)) {
        label->setObjectName("loadZLabel");
    }

    m_unitCombo = new QComboBox(this);
    m_unitCombo->setEditable(false);
    form->addRow(zh(u8"单位:"), m_unitCombo);

    connect(m_typeCombo, &QComboBox::currentTextChanged, this, [this, surfaceForceHintLabel, tractionHintLabel]() {
        const LoadType type = selectedLoadType(m_typeCombo);
        const QString currentDefault = defaultFieldName(type);
        if (isDefaultFieldName(m_fieldNameEdit->text())) {
            m_fieldNameEdit->setText(currentDefault);
        }
        updateUnitItems();
        const bool vectorLoad = isVectorLoadType(type);
        m_yValueSpin->setVisible(vectorLoad);
        m_zValueSpin->setVisible(vectorLoad);
        if (QWidget *label = findChild<QWidget *>("loadYLabel")) {
            label->setVisible(vectorLoad);
        }
        if (QWidget *label = findChild<QWidget *>("loadZLabel")) {
            label->setVisible(vectorLoad);
        }
        surfaceForceHintLabel->setVisible(type == LoadType::SurfaceForce);
        tractionHintLabel->setVisible(type == LoadType::Traction);
    });
    m_fieldNameEdit->setText(defaultFieldName(selectedLoadType(m_typeCombo)));
    updateUnitItems();
    const bool vectorLoad = isVectorLoadType(selectedLoadType(m_typeCombo));
    m_yValueSpin->setVisible(vectorLoad);
    m_zValueSpin->setVisible(vectorLoad);
    if (QWidget *label = findChild<QWidget *>("loadYLabel")) {
        label->setVisible(vectorLoad);
    }
    if (QWidget *label = findChild<QWidget *>("loadZLabel")) {
        label->setVisible(vectorLoad);
    }
    surfaceForceHintLabel->setVisible(selectedLoadType(m_typeCombo) == LoadType::SurfaceForce);
    tractionHintLabel->setVisible(selectedLoadType(m_typeCombo) == LoadType::Traction);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"验证"), zh(u8"载荷名称不能为空。"));
            return;
        }
        if (selectedLoadType(m_typeCombo) != LoadType::Gravity && selectedBoundaryConditionId().isEmpty()) {
            QMessageBox::warning(this, zh(u8"验证"), zh(u8"创建该载荷前，请先选择一个边界条件。"));
            return;
        }
        if (m_unitCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"验证"), zh(u8"请选择载荷单位。"));
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
    ld.value.kind = isVectorLoadType(ld.type) ? LoadValueKind::Vector3 : LoadValueKind::Scalar;
    ld.value.x = m_valueSpin->value();
    ld.value.y = m_yValueSpin->value();
    ld.value.z = m_zValueSpin->value();
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
    m_yValueSpin->setValue(ld.value.y);
    m_zValueSpin->setValue(ld.value.z);
    updateUnitItems();
    const bool vectorLoad = isVectorLoadType(selectedLoadType(m_typeCombo));
    m_yValueSpin->setVisible(vectorLoad);
    m_zValueSpin->setVisible(vectorLoad);
    if (QWidget *label = findChild<QWidget *>("loadYLabel")) {
        label->setVisible(vectorLoad);
    }
    if (QWidget *label = findChild<QWidget *>("loadZLabel")) {
        label->setVisible(vectorLoad);
    }
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
