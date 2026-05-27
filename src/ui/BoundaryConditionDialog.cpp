#include "BoundaryConditionDialog.h"

#include "geometry/FaceGroup.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
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
    m_typeCombo->addItem(zh(u8"壁面"), static_cast<int>(BoundaryConditionType::Wall));
    m_typeCombo->addItem(zh(u8"速度入口"), static_cast<int>(BoundaryConditionType::VelocityInlet));
    m_typeCombo->addItem(zh(u8"压力入口"), static_cast<int>(BoundaryConditionType::PressureInlet));
    m_typeCombo->addItem(zh(u8"压力出口"), static_cast<int>(BoundaryConditionType::PressureOutlet));
    m_typeCombo->addItem(zh(u8"对称"), static_cast<int>(BoundaryConditionType::Symmetry));
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
