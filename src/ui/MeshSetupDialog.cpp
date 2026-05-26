#include "MeshSetupDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QVBoxLayout>

namespace
{
void configureSizeSpinBox(QDoubleSpinBox *spinBox)
{
    spinBox->setRange(0.0, 1.0e9);
    spinBox->setDecimals(6);
    spinBox->setSingleStep(1.0);
    spinBox->setSpecialValueText("Disabled");
}
}

MeshSetupDialog::MeshSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Mesh Setup");
    setupUi();
}

void MeshSetupDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_elementTypeCombo = new QComboBox(this);
    m_elementTypeCombo->addItem(displayName(MeshElementType::Tetra4), toString(MeshElementType::Tetra4));
    const int tet10Index = m_elementTypeCombo->count();
    m_elementTypeCombo->addItem("Tet10 - Quadratic tetrahedron (not supported yet)", "tetra10");
    if (auto *model = qobject_cast<QStandardItemModel *>(m_elementTypeCombo->model())) {
        if (QStandardItem *item = model->item(tet10Index)) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
    form->addRow("Element type:", m_elementTypeCombo);

    m_autoSizeCheckBox = new QCheckBox(this);
    form->addRow("Auto size:", m_autoSizeCheckBox);

    m_minimumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_minimumSizeSpin);
    form->addRow("Minimum size:", m_minimumSizeSpin);

    m_maximumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_maximumSizeSpin);
    form->addRow("Maximum size:", m_maximumSizeSpin);

    connect(m_autoSizeCheckBox, &QCheckBox::toggled, this, [this]() {
        updateSizeControlState();
    });

    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (!m_autoSizeCheckBox->isChecked()
            && m_minimumSizeSpin->value() > 0.0
            && m_maximumSizeSpin->value() > 0.0
            && m_minimumSizeSpin->value() > m_maximumSizeSpin->value()) {
            QMessageBox::warning(
                this,
                "Validation",
                "Minimum mesh size cannot be greater than maximum mesh size."
            );
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

MeshSetup MeshSetupDialog::meshSetup() const
{
    MeshSetup setup = m_currentSetup;
    setup.elementType = selectedElementType();
    setup.autoSize = m_autoSizeCheckBox->isChecked();
    setup.minimumSize = setup.autoSize ? 0.0 : m_minimumSizeSpin->value();
    setup.maximumSize = setup.autoSize ? 0.0 : m_maximumSizeSpin->value();
    return setup;
}

void MeshSetupDialog::setMeshSetup(const MeshSetup &meshSetup)
{
    m_currentSetup = meshSetup;
    setElementType(meshSetup.elementType);
    m_autoSizeCheckBox->setChecked(meshSetup.autoSize);
    m_minimumSizeSpin->setValue(meshSetup.minimumSize);
    m_maximumSizeSpin->setValue(meshSetup.maximumSize);
    updateSizeControlState();
}

void MeshSetupDialog::updateSizeControlState()
{
    const bool manual = !m_autoSizeCheckBox->isChecked();
    m_minimumSizeSpin->setEnabled(manual);
    m_maximumSizeSpin->setEnabled(manual);
}

void MeshSetupDialog::setElementType(MeshElementType elementType)
{
    const int index = m_elementTypeCombo->findData(toString(elementType));
    m_elementTypeCombo->setCurrentIndex(index >= 0 ? index : 0);
}

MeshElementType MeshSetupDialog::selectedElementType() const
{
    const QString value = m_elementTypeCombo->currentData().toString();
    Q_UNUSED(value);
    return MeshElementType::Tetra4;
}

std::optional<MeshSetup> MeshSetupDialog::editMeshSetup(
    QWidget *parent,
    const MeshSetup &current
)
{
    MeshSetupDialog dialog(parent);
    dialog.setMeshSetup(current);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.meshSetup();
    }
    return std::nullopt;
}
