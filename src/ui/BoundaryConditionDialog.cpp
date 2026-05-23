#include "BoundaryConditionDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

namespace
{
QString defaultFaceGroupName()
{
    return "Default";
}
}

BoundaryConditionDialog::BoundaryConditionDialog(
    BoundaryConditionDialogOptions options,
    QWidget *parent
)
    : QDialog(parent)
    , m_options(std::move(options))
{
    setWindowTitle("Create Boundary Condition");
    setupUi();
}

void BoundaryConditionDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("e.g. Inlet1, Outlet, Wall");
    form->addRow("Name:", m_nameEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Wall", static_cast<int>(BoundaryConditionType::Wall));
    m_typeCombo->addItem("Velocity Inlet", static_cast<int>(BoundaryConditionType::VelocityInlet));
    m_typeCombo->addItem("Pressure Inlet", static_cast<int>(BoundaryConditionType::PressureInlet));
    m_typeCombo->addItem("Pressure Outlet", static_cast<int>(BoundaryConditionType::PressureOutlet));
    m_typeCombo->addItem("Symmetry", static_cast<int>(BoundaryConditionType::Symmetry));
    form->addRow("Type:", m_typeCombo);

    m_geometryNameCombo = new QComboBox(this);
    m_geometryNameCombo->addItems(m_options.geometryNames);
    m_geometryNameCombo->setEditable(m_options.geometryNames.isEmpty());
    if (m_geometryNameCombo->isEditable()) {
        m_geometryNameCombo->setPlaceholderText("e.g. Pipe");
    }
    form->addRow("Geometry:", m_geometryNameCombo);

    m_faceGroupNameCombo = new QComboBox(this);
    m_faceGroupNameCombo->setEditable(true);
    form->addRow("Face Group:", m_faceGroupNameCombo);

    m_materialIdCombo = new QComboBox(this);
    m_materialIdCombo->addItems(m_options.materialIds);
    m_materialIdCombo->setEditable(true);
    if (m_materialIdCombo->isEditable()) {
        m_materialIdCombo->setPlaceholderText("e.g. water, air");
    }
    form->addRow("Material ID:", m_materialIdCombo);

    connect(m_geometryNameCombo, &QComboBox::currentTextChanged, this, [this](const QString &geometryName) {
        updateFaceGroupItems(geometryName);
    });
    updateFaceGroupItems(m_geometryNameCombo->currentText());

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Boundary condition name cannot be empty.");
            return;
        }
        if (m_geometryNameCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Please choose a target geometry.");
            return;
        }
        if (m_faceGroupNameCombo->currentText().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Please choose or enter a face group.");
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
    bc.target.faceGroupName = m_faceGroupNameCombo->currentText().trimmed();
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
    setComboCurrentText(m_faceGroupNameCombo, bc.target.faceGroupName);
    setComboCurrentText(m_materialIdCombo, bc.materialId);
}

void BoundaryConditionDialog::updateFaceGroupItems(const QString &geometryName)
{
    const QString currentText = m_faceGroupNameCombo->currentText();
    QStringList faceGroups = m_options.faceGroupsByGeometry.value(geometryName);
    if (faceGroups.isEmpty()) {
        faceGroups.append(defaultFaceGroupName());
    }

    m_faceGroupNameCombo->blockSignals(true);
    m_faceGroupNameCombo->clear();
    m_faceGroupNameCombo->addItems(faceGroups);
    if (!currentText.trimmed().isEmpty()) {
        setComboCurrentText(m_faceGroupNameCombo, currentText.trimmed());
    }
    m_faceGroupNameCombo->blockSignals(false);
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
    dlg.setWindowTitle("Create Boundary Condition");
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
    dlg.setWindowTitle("Edit Boundary Condition");
    dlg.setBoundaryCondition(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.boundaryCondition();
    }
    return std::nullopt;
}
