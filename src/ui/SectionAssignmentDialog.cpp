#include "SectionAssignmentDialog.h"

#include "mesh/MeshData.h"
#include "mesh/MshReader.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString displayNameWithId(const QString &name, const QString &id)
{
    const QString trimmedName = name.trimmed();
    const QString trimmedId = id.trimmed();
    if (trimmedName.isEmpty() || trimmedName == trimmedId) {
        return trimmedId;
    }
    return QString("%1 (%2)").arg(trimmedName, trimmedId);
}

QString sanitizedIdPart(QString text)
{
    text = text.trimmed().toLower();
    for (qsizetype i = 0; i < text.size(); ++i) {
        if (!text.at(i).isLetterOrNumber()) {
            text[i] = '_';
        }
    }
    while (text.contains("__")) {
        text.replace("__", "_");
    }
    while (text.startsWith('_')) {
        text.remove(0, 1);
    }
    while (text.endsWith('_')) {
        text.chop(1);
    }
    return text;
}

void addEditableElementSetDefaults(QComboBox *combo)
{
    combo->setEditable(true);
    combo->addItem("EALL");
}

QString absolutePath(const QString &rootPath, const QString &path)
{
    return QFileInfo(path).isAbsolute() ? path : QDir(rootPath).filePath(path);
}

const MeshObject *findMeshByName(const QVector<MeshObject> &meshes, const QString &meshName)
{
    for (const MeshObject &mesh : meshes) {
        if (mesh.name == meshName) {
            return &mesh;
        }
    }
    return nullptr;
}

QString physicalVolumeDisplayName(const MeshPhysicalGroup &physicalGroup)
{
    const QString name = physicalGroup.name.trimmed();
    return name.isEmpty()
        ? QString("PhysicalVolume_%1").arg(physicalGroup.tag)
        : name;
}

QStringList sectionElementSetNamesForMesh(
    const SectionAssignmentDialogOptions &options,
    const MeshObject &mesh,
    QString *readWarning
)
{
    QStringList names;
    const QString meshPath = absolutePath(options.projectRootPath, mesh.mshFile);
    if (mesh.mshFile.trimmed().isEmpty() || !QFileInfo::exists(meshPath)) {
        if (readWarning) {
            *readWarning = QString::fromUtf8(u8"当前网格没有可读取的 MSH 文件，只能使用 EALL。");
        }
        return names;
    }

    MeshData meshData;
    meshData.name = mesh.name;
    meshData.sourceGeometryName = mesh.sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshPath, meshData, &errorMessage)) {
        if (readWarning) {
            *readWarning = QString::fromUtf8(u8"读取 MSH 失败，只能使用 EALL：") + errorMessage;
        }
        return names;
    }

    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroup.dimension != 3) {
            continue;
        }
        names.append(physicalVolumeDisplayName(physicalGroup));
        if (!physicalGroup.name.trimmed().isEmpty()) {
            names.append(QString("tag_%1").arg(physicalGroup.tag));
        }
    }
    names.removeDuplicates();
    std::sort(names.begin(), names.end(), [](const QString &left, const QString &right) {
        return QString::localeAwareCompare(left, right) < 0;
    });
    return names;
}
}

SectionAssignmentDialog::SectionAssignmentDialog(SectionAssignmentDialogOptions options, QWidget *parent)
    : QDialog(parent)
    , m_options(std::move(options))
{
    setWindowTitle(zh(u8"创建材料分区"));
    setupUi();
}

void SectionAssignmentDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(zh(u8"例如：钢材分区 / 外壳材料"));
    form->addRow(zh(u8"名称:"), m_nameEdit);

    m_materialCombo = new QComboBox(this);
    for (const Material &material : m_options.solidMaterials) {
        if (material.domain != MaterialDomain::Solid) {
            continue;
        }
        m_materialCombo->addItem(displayNameWithId(material.name, material.id), material.id);
    }
    form->addRow(zh(u8"材料:"), m_materialCombo);

    m_geometryCombo = new QComboBox(this);
    for (const GeometryObject &geometry : m_options.geometries) {
        m_geometryCombo->addItem(geometry.name, geometry.name);
    }
    setComboCurrentData(m_geometryCombo, m_options.defaultGeometryName);
    form->addRow(zh(u8"几何体:"), m_geometryCombo);

    m_meshCombo = new QComboBox(this);
    for (const MeshObject &mesh : m_options.meshes) {
        QString label = mesh.name;
        if (!mesh.sourceGeometryName.trimmed().isEmpty()) {
            label += QString(" - %1").arg(mesh.sourceGeometryName);
        }
        m_meshCombo->addItem(label, mesh.name);
    }
    setComboCurrentData(m_meshCombo, m_options.defaultMeshName);
    connect(m_meshCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        const QString meshName = selectedData(m_meshCombo);
        for (const MeshObject &mesh : m_options.meshes) {
            if (mesh.name == meshName && !mesh.sourceGeometryName.trimmed().isEmpty()) {
                setComboCurrentData(m_geometryCombo, mesh.sourceGeometryName);
                break;
            }
        }
        refreshElementSetOptionsForSelectedMesh();
    });
    form->addRow(zh(u8"网格:"), m_meshCombo);

    m_elementSetCombo = new QComboBox(this);
    addEditableElementSetDefaults(m_elementSetCombo);
    m_elementSetCombo->setCurrentText("EALL");
    form->addRow(zh(u8"单元集:"), m_elementSetCombo);

    m_elementSetHintLabel = new QLabel(this);
    m_elementSetHintLabel->setWordWrap(true);
    m_elementSetHintLabel->setStyleSheet("color: #64748b;");
    form->addRow(QString(), m_elementSetHintLabel);
    refreshElementSetOptionsForSelectedMesh();

    m_enabledCheck = new QCheckBox(zh(u8"启用"), this);
    m_enabledCheck->setChecked(true);
    form->addRow(m_enabledCheck);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"材料分区名称不能为空。"));
            return;
        }
        if (selectedData(m_materialCombo).isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"请先选择一个固体材料。"));
            return;
        }
        if (selectedData(m_meshCombo).isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"请先选择一个网格。"));
            return;
        }
        if (m_elementSetCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, zh(u8"校验"), zh(u8"单元集不能为空，默认可使用 EALL。"));
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void SectionAssignmentDialog::refreshElementSetOptionsForSelectedMesh()
{
    if (!m_elementSetCombo) {
        return;
    }

    const QString previousText = m_elementSetCombo->currentText().trimmed();
    m_elementSetCombo->blockSignals(true);
    m_elementSetCombo->clear();
    addEditableElementSetDefaults(m_elementSetCombo);

    QString hint;
    if (const MeshObject *mesh = findMeshByName(m_options.meshes, selectedData(m_meshCombo))) {
        const QStringList names = sectionElementSetNamesForMesh(m_options, *mesh, &hint);
        for (const QString &name : names) {
            if (m_elementSetCombo->findText(name, Qt::MatchFixedString) < 0) {
                m_elementSetCombo->addItem(name);
            }
        }
        if (hint.isEmpty()) {
            hint = names.isEmpty()
                ? zh(u8"当前网格未检测到 3D Physical Volume；材料分区只能使用 EALL，表示全部体单元。")
                : zh(u8"已从当前 MSH 检测到 3D Physical Volume，可直接选择作为材料分区单元集。");
        }
    } else {
        hint = zh(u8"请先选择网格；未选择网格时只能使用 EALL。");
    }

    if (!previousText.isEmpty()) {
        const int existingIndex = m_elementSetCombo->findText(previousText, Qt::MatchFixedString);
        if (existingIndex >= 0) {
            m_elementSetCombo->setCurrentIndex(existingIndex);
        } else {
            m_elementSetCombo->setCurrentText(previousText);
        }
    } else {
        m_elementSetCombo->setCurrentText("EALL");
    }
    m_elementSetCombo->blockSignals(false);

    if (m_elementSetHintLabel) {
        m_elementSetHintLabel->setText(hint);
    }
}

SectionAssignment SectionAssignmentDialog::sectionAssignment() const
{
    SectionAssignment sectionAssignment;
    sectionAssignment.id = m_id.trimmed().isEmpty() ? generatedId() : sanitizedIdPart(m_id);
    sectionAssignment.name = m_nameEdit->text().trimmed();
    sectionAssignment.materialId = selectedData(m_materialCombo);
    sectionAssignment.geometryName = selectedData(m_geometryCombo);
    sectionAssignment.meshName = selectedData(m_meshCombo);
    sectionAssignment.elementSetName = m_elementSetCombo->currentText().trimmed();
    if (sectionAssignment.elementSetName.isEmpty()) {
        sectionAssignment.elementSetName = "EALL";
    }
    sectionAssignment.enabled = m_enabledCheck->isChecked();
    return sectionAssignment;
}

void SectionAssignmentDialog::setSectionAssignment(const SectionAssignment &sectionAssignment)
{
    m_id = sectionAssignment.id;
    m_nameEdit->setText(sectionAssignment.name);
    setComboCurrentData(m_materialCombo, sectionAssignment.materialId);
    setComboCurrentData(m_geometryCombo, sectionAssignment.geometryName);
    setComboCurrentData(m_meshCombo, sectionAssignment.meshName);
    refreshElementSetOptionsForSelectedMesh();
    m_elementSetCombo->setCurrentText(sectionAssignment.elementSetName.trimmed().isEmpty()
        ? QString("EALL")
        : sectionAssignment.elementSetName);
    m_enabledCheck->setChecked(sectionAssignment.enabled);
}

std::optional<SectionAssignment> SectionAssignmentDialog::createSectionAssignment(
    QWidget *parent,
    SectionAssignmentDialogOptions options
)
{
    SectionAssignmentDialog dialog(std::move(options), parent);
    dialog.setWindowTitle(zh(u8"创建材料分区"));
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.sectionAssignment();
    }
    return std::nullopt;
}

std::optional<SectionAssignment> SectionAssignmentDialog::editSectionAssignment(
    QWidget *parent,
    const SectionAssignment &existing,
    SectionAssignmentDialogOptions options
)
{
    SectionAssignmentDialog dialog(std::move(options), parent);
    dialog.setWindowTitle(zh(u8"编辑材料分区"));
    dialog.setSectionAssignment(existing);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.sectionAssignment();
    }
    return std::nullopt;
}

void SectionAssignmentDialog::setComboCurrentData(QComboBox *combo, const QString &value)
{
    if (!combo || value.trimmed().isEmpty()) {
        return;
    }
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemData(i).toString() == value) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    combo->addItem(value, value);
    combo->setCurrentIndex(combo->count() - 1);
}

QString SectionAssignmentDialog::selectedData(const QComboBox *combo) const
{
    if (!combo) {
        return {};
    }
    return combo->currentData().toString().trimmed();
}

QString SectionAssignmentDialog::generatedId() const
{
    const QString materialPart = sanitizedIdPart(selectedData(m_materialCombo));
    const QString meshPart = sanitizedIdPart(selectedData(m_meshCombo));
    QStringList parts{"section"};
    if (!materialPart.isEmpty()) {
        parts.append(materialPart);
    }
    if (!meshPart.isEmpty()) {
        parts.append(meshPart);
    }
    return parts.join('_');
}
