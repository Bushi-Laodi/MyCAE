#include "BoundaryConditionDialog.h"

#include "geometry/FaceGroup.h"

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

QString boundaryTypeLabel(BoundaryConditionType type)
{
    switch (type) {
    case BoundaryConditionType::FixedSupport:
        return zh(u8"固定约束");
    case BoundaryConditionType::Displacement:
        return zh(u8"指定位移");
    case BoundaryConditionType::LoadTarget:
        return zh(u8"载荷作用面");
    case BoundaryConditionType::Wall:
        return zh(u8"壁面 / 固定支撑");
    case BoundaryConditionType::VelocityInlet:
        return zh(u8"速度入口");
    case BoundaryConditionType::PressureInlet:
        return zh(u8"压力入口");
    case BoundaryConditionType::PressureOutlet:
        return zh(u8"压力出口");
    case BoundaryConditionType::Symmetry:
        return zh(u8"对称");
    case BoundaryConditionType::SymmetryStructural:
        return zh(u8"结构对称约束");
    case BoundaryConditionType::Unknown:
        return zh(u8"未知");
    }
    return zh(u8"未知");
}

std::vector<BoundaryConditionType> defaultBoundaryTypes()
{
    return {
        BoundaryConditionType::FixedSupport,
        BoundaryConditionType::Displacement,
        BoundaryConditionType::LoadTarget,
        BoundaryConditionType::Wall,
        BoundaryConditionType::VelocityInlet,
        BoundaryConditionType::PressureInlet,
        BoundaryConditionType::PressureOutlet,
        BoundaryConditionType::Symmetry,
        BoundaryConditionType::SymmetryStructural
    };
}
}

BoundaryConditionDialog::BoundaryConditionDialog(
    BoundaryConditionDialogOptions options,
    QWidget *parent
)
    : QDialog(parent)
    , m_options(std::move(options))
{
    setWindowTitle(zh(u8"创建边界条件"));
    setupUi();
}

void BoundaryConditionDialog::setupUi()
{
    for (auto it = m_options.faceGroupsByGeometry.cbegin(); it != m_options.faceGroupsByGeometry.cend(); ++it) {
        for (const QString &faceGroupId : it.value()) {
            m_faceGroupNamesById.insert(faceGroupId, FaceGroups::nameFromId(faceGroupId));
        }
    }

    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(zh(u8"例如：入口、出口、壁面"));
    form->addRow(zh(u8"名称:"), m_nameEdit);

    m_typeCombo = new QComboBox(this);
    const std::vector<BoundaryConditionType> types =
        m_options.allowedTypes.empty() ? defaultBoundaryTypes() : m_options.allowedTypes;
    for (const BoundaryConditionType type : types) {
        m_typeCombo->addItem(boundaryTypeLabel(type), static_cast<int>(type));
    }
    m_typeCombo->setEnabled(types.size() > 1);
    form->addRow(zh(u8"类型:"), m_typeCombo);

    m_geometryNameCombo = new QComboBox(this);
    m_geometryNameCombo->addItems(m_options.geometryNames);
    m_geometryNameCombo->setEditable(m_options.geometryNames.isEmpty());
    if (m_geometryNameCombo->isEditable()) {
        m_geometryNameCombo->setPlaceholderText(zh(u8"例如：管道"));
    }
    if (!m_options.defaultGeometryName.trimmed().isEmpty()) {
        setComboCurrentText(m_geometryNameCombo, m_options.defaultGeometryName);
    }
    form->addRow(zh(u8"几何:"), m_geometryNameCombo);

    m_faceGroupNameCombo = new QComboBox(this);
    m_faceGroupNameCombo->setEditable(true);
    form->addRow(zh(u8"面组:"), m_faceGroupNameCombo);

    m_materialIdCombo = new QComboBox(this);
    m_materialIdCombo->addItems(m_options.materialIds);
    m_materialIdCombo->setEditable(true);
    if (m_materialIdCombo->isEditable()) {
        m_materialIdCombo->setPlaceholderText(zh(u8"例如：water, air（材料 ID）"));
    }
    form->addRow(zh(u8"材料 ID:"), m_materialIdCombo);

    m_uxCheck = new QCheckBox("Ux", this);
    m_uyCheck = new QCheckBox("Uy", this);
    m_uzCheck = new QCheckBox("Uz", this);
    m_uxCheck->setChecked(true);
    m_uyCheck->setChecked(true);
    m_uzCheck->setChecked(true);

    m_uxSpin = new QDoubleSpinBox(this);
    m_uySpin = new QDoubleSpinBox(this);
    m_uzSpin = new QDoubleSpinBox(this);
    for (QDoubleSpinBox *spin : {m_uxSpin, m_uySpin, m_uzSpin}) {
        spin->setRange(-1e9, 1e9);
        spin->setDecimals(6);
    }
    m_displacementUnitCombo = new QComboBox(this);
    m_displacementUnitCombo->addItems({"m", "mm"});
    form->addRow(m_uxCheck, m_uxSpin);
    form->addRow(m_uyCheck, m_uySpin);
    form->addRow(m_uzCheck, m_uzSpin);
    form->addRow(zh(u8"位移单位:"), m_displacementUnitCombo);
    if (QWidget *label = form->labelForField(m_displacementUnitCombo)) {
        label->setObjectName("displacementUnitLabel");
    }

    connect(m_typeCombo, &QComboBox::currentIndexChanged, this, [this]() {
        updateDisplacementEditors();
    });
    updateDisplacementEditors();

    connect(m_geometryNameCombo, &QComboBox::currentTextChanged, this, [this](const QString &geometryName) {
        updateFaceGroupItems(geometryName);
    });
    updateFaceGroupItems(m_geometryNameCombo->currentText());
    if (!m_options.defaultFaceGroupId.trimmed().isEmpty()) {
        setComboCurrentText(m_faceGroupNameCombo, m_options.defaultFaceGroupId);
    }

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"边界条件名称不能为空。"));
            return;
        }
        if (m_geometryNameCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"请选择目标几何。"));
            return;
        }
        if (m_faceGroupNameCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"请选择或输入面组。"));
            return;
        }
        const auto type = static_cast<BoundaryConditionType>(m_typeCombo->currentData().toInt());
        if (type == BoundaryConditionType::Displacement
                && !m_uxCheck->isChecked()
                && !m_uyCheck->isChecked()
                && !m_uzCheck->isChecked()) {
            QMessageBox::warning(this, zh(u8"验证"), zh(u8"指定位移至少需要启用一个自由度。"));
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

BoundaryCondition BoundaryConditionDialog::boundaryCondition() const
{
    BoundaryCondition bc;
    bc.id = m_nameEdit->text().trimmed().toLower().replace(' ', '_');
    bc.name = m_nameEdit->text().trimmed();
    bc.type = static_cast<BoundaryConditionType>(m_typeCombo->currentData().toInt());
    bc.target.geometryName = m_geometryNameCombo->currentText().trimmed();
    bc.target.faceGroupId = selectedFaceGroupId();
    bc.target.faceGroupName = m_faceGroupNamesById.value(
        bc.target.faceGroupId,
        m_faceGroupNameCombo->currentText().trimmed()
    );
    bc.displacement.uxEnabled = m_uxCheck->isChecked();
    bc.displacement.uyEnabled = m_uyCheck->isChecked();
    bc.displacement.uzEnabled = m_uzCheck->isChecked();
    bc.displacement.ux = m_uxSpin->value();
    bc.displacement.uy = m_uySpin->value();
    bc.displacement.uz = m_uzSpin->value();
    bc.displacement.unit = m_displacementUnitCombo->currentText().trimmed();
    bc.materialId = m_materialIdCombo->currentText().trimmed();
    return bc;
}

void BoundaryConditionDialog::setBoundaryCondition(const BoundaryCondition &bc)
{
    m_nameEdit->setText(bc.name);
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toInt() == static_cast<int>(bc.type)) {
            m_typeCombo->setCurrentIndex(i);
            break;
        }
    }
    setComboCurrentText(m_geometryNameCombo, bc.target.geometryName);
    updateFaceGroupItems(m_geometryNameCombo->currentText());
    setComboCurrentText(
        m_faceGroupNameCombo,
        bc.target.faceGroupId.isEmpty() ? bc.target.faceGroupName : bc.target.faceGroupId
    );
    m_uxCheck->setChecked(bc.displacement.uxEnabled);
    m_uyCheck->setChecked(bc.displacement.uyEnabled);
    m_uzCheck->setChecked(bc.displacement.uzEnabled);
    m_uxSpin->setValue(bc.displacement.ux);
    m_uySpin->setValue(bc.displacement.uy);
    m_uzSpin->setValue(bc.displacement.uz);
    setComboCurrentText(m_displacementUnitCombo, bc.displacement.unit);
    updateDisplacementEditors();
    setComboCurrentText(m_materialIdCombo, bc.materialId);
}

void BoundaryConditionDialog::updateFaceGroupItems(const QString &geometryName)
{
    const QString currentText = m_faceGroupNameCombo->currentText();
    QStringList faceGroupLabels = m_options.faceGroupsByGeometry.value(geometryName);
    if (faceGroupLabels.isEmpty()) {
        faceGroupLabels.append(FaceGroups::defaultName());
    }

    m_faceGroupNameCombo->blockSignals(true);
    m_faceGroupNameCombo->clear();
    m_faceGroupNameCombo->addItems(faceGroupLabels);
    if (!currentText.trimmed().isEmpty()) {
        setComboCurrentText(m_faceGroupNameCombo, currentText.trimmed());
    }
    m_faceGroupNameCombo->blockSignals(false);
}

void BoundaryConditionDialog::updateDisplacementEditors()
{
    const auto type = static_cast<BoundaryConditionType>(m_typeCombo->currentData().toInt());
    const bool enabled = type == BoundaryConditionType::Displacement;
    for (QWidget *widget : {
             static_cast<QWidget *>(m_uxCheck),
             static_cast<QWidget *>(m_uyCheck),
             static_cast<QWidget *>(m_uzCheck),
             static_cast<QWidget *>(m_uxSpin),
             static_cast<QWidget *>(m_uySpin),
             static_cast<QWidget *>(m_uzSpin),
             static_cast<QWidget *>(m_displacementUnitCombo)
         }) {
        widget->setVisible(enabled);
    }
    if (QWidget *label = findChild<QWidget *>("displacementUnitLabel")) {
        label->setVisible(enabled);
    }
}

QString BoundaryConditionDialog::selectedFaceGroupId() const
{
    const QString text = m_faceGroupNameCombo->currentText().trimmed();
    if (m_faceGroupNamesById.contains(text)) {
        return text;
    }
    if (text.contains('.')) {
        return text;
    }
    return FaceGroups::makeId(m_geometryNameCombo->currentText(), text);
}

void BoundaryConditionDialog::setComboCurrentText(QComboBox *combo, const QString &text)
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

std::optional<BoundaryCondition> BoundaryConditionDialog::createBoundaryCondition(
    QWidget *parent,
    BoundaryConditionDialogOptions options
)
{
    BoundaryConditionDialog dlg(std::move(options), parent);
    dlg.setWindowTitle(zh(u8"创建边界条件"));
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.boundaryCondition();
    }
    return std::nullopt;
}

std::optional<BoundaryCondition> BoundaryConditionDialog::editBoundaryCondition(
    QWidget *parent,
    const BoundaryCondition &existing,
    BoundaryConditionDialogOptions options
)
{
    BoundaryConditionDialog dlg(std::move(options), parent);
    dlg.setWindowTitle(zh(u8"编辑边界条件"));
    dlg.setBoundaryCondition(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.boundaryCondition();
    }
    return std::nullopt;
}
