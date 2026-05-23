#include "BoundaryConditionDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

BoundaryConditionDialog::BoundaryConditionDialog(QWidget *parent)
    : QDialog(parent)
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

    m_geometryNameEdit = new QLineEdit(this);
    m_geometryNameEdit->setPlaceholderText("e.g. Pipe");
    form->addRow("Geometry Name:", m_geometryNameEdit);

    m_faceGroupNameEdit = new QLineEdit(this);
    m_faceGroupNameEdit->setPlaceholderText("e.g. Inlet1, Default");
    form->addRow("Face Group:", m_faceGroupNameEdit);

    m_materialIdEdit = new QLineEdit(this);
    m_materialIdEdit->setPlaceholderText("e.g. water, air");
    form->addRow("Material ID:", m_materialIdEdit);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Boundary condition name cannot be empty.");
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
    bc.target.geometryName = m_geometryNameEdit->text().trimmed();
    bc.target.faceGroupName = m_faceGroupNameEdit->text().trimmed();
    bc.materialId = m_materialIdEdit->text().trimmed();
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
    m_geometryNameEdit->setText(bc.target.geometryName);
    m_faceGroupNameEdit->setText(bc.target.faceGroupName);
    m_materialIdEdit->setText(bc.materialId);
}

std::optional<BoundaryCondition> BoundaryConditionDialog::createBoundaryCondition(QWidget *parent)
{
    BoundaryConditionDialog dlg(parent);
    dlg.setWindowTitle("Create Boundary Condition");
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.boundaryCondition();
    }
    return std::nullopt;
}

std::optional<BoundaryCondition> BoundaryConditionDialog::editBoundaryCondition(QWidget *parent, const BoundaryCondition &existing)
{
    BoundaryConditionDialog dlg(parent);
    dlg.setWindowTitle("Edit Boundary Condition");
    dlg.setBoundaryCondition(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.boundaryCondition();
    }
    return std::nullopt;
}
