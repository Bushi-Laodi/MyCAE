#include "LoadDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

LoadDialog::LoadDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Create Load");
    setupUi();
}

void LoadDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("e.g. Inlet1 velocity, Inlet2 pressure");
    form->addRow("Name:", m_nameEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("Velocity", static_cast<int>(LoadType::Velocity));
    m_typeCombo->addItem("Pressure", static_cast<int>(LoadType::Pressure));
    m_typeCombo->addItem("Body Force", static_cast<int>(LoadType::BodyForce));
    form->addRow("Type:", m_typeCombo);

    m_boundaryConditionIdEdit = new QLineEdit(this);
    m_boundaryConditionIdEdit->setPlaceholderText("e.g. bc_inlet1");
    form->addRow("Boundary Condition ID:", m_boundaryConditionIdEdit);

    m_fieldNameEdit = new QLineEdit(this);
    m_fieldNameEdit->setPlaceholderText("e.g. U (velocity), p (pressure)");
    form->addRow("Field Name:", m_fieldNameEdit);

    m_valueSpin = new QDoubleSpinBox(this);
    m_valueSpin->setRange(-1e9, 1e9);
    m_valueSpin->setDecimals(4);
    form->addRow("Value:", m_valueSpin);

    m_unitEdit = new QLineEdit(this);
    m_unitEdit->setPlaceholderText("e.g. m/s, Pa");
    form->addRow("Unit:", m_unitEdit);

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Validation", "Load name cannot be empty.");
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
    ld.boundaryConditionId = m_boundaryConditionIdEdit->text().trimmed();
    ld.fieldName = m_fieldNameEdit->text().trimmed();
    ld.value.kind = LoadValueKind::Scalar;
    ld.value.x = m_valueSpin->value();
    ld.value.unit = m_unitEdit->text().trimmed();
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
    m_boundaryConditionIdEdit->setText(ld.boundaryConditionId);
    m_fieldNameEdit->setText(ld.fieldName);
    m_valueSpin->setValue(ld.value.x);
    m_unitEdit->setText(ld.value.unit);
}

Load LoadDialog::createLoad(QWidget *parent)
{
    LoadDialog dlg(parent);
    dlg.setWindowTitle("Create Load");
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.load();
    }
    return {};
}

Load LoadDialog::editLoad(QWidget *parent, const Load &existing)
{
    LoadDialog dlg(parent);
    dlg.setWindowTitle("Edit Load");
    dlg.setLoad(existing);
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.load();
    }
    return {};
}
