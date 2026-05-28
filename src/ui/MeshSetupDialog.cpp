#include "MeshSetupDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

void configureSizeSpinBox(QDoubleSpinBox *spinBox)
{
    spinBox->setRange(0.0, 1.0e9);
    spinBox->setDecimals(6);
    spinBox->setSingleStep(1.0);
    spinBox->setSpecialValueText(zh(u8"禁用"));
}
}

MeshSetupDialog::MeshSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"网格设置"));
    setupUi();
}

void MeshSetupDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_elementTypeCombo = new QComboBox(this);
    m_elementTypeCombo->addItem(zh(u8"Tet4 - 线性四面体"), toString(MeshElementType::Tetra4));
    m_elementTypeCombo->addItem(zh(u8"Tet10 - 二次四面体"), toString(MeshElementType::Tetra10));
    form->addRow(zh(u8"单元类型:"), m_elementTypeCombo);
    connect(m_elementTypeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateSizePreview();
    });

    m_autoSizeCheckBox = new QCheckBox(this);
    form->addRow(zh(u8"自动尺寸:"), m_autoSizeCheckBox);

    m_minimumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_minimumSizeSpin);
    form->addRow(zh(u8"最小尺寸:"), m_minimumSizeSpin);

    m_maximumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_maximumSizeSpin);
    form->addRow(zh(u8"最大尺寸:"), m_maximumSizeSpin);

    connect(m_autoSizeCheckBox, &QCheckBox::toggled, this, [this]() {
        updateSizeControlState();
        updateSizePreview();
    });
    connect(m_minimumSizeSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this]() {
        updateSizePreview();
    });
    connect(m_maximumSizeSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this]() {
        updateSizePreview();
    });

    mainLayout->addLayout(form);

    m_sizePreviewLabel = new QLabel(this);
    m_sizePreviewLabel->setWordWrap(true);
    m_sizePreviewLabel->setObjectName("mesh.setup.sizePreview");
    m_sizePreviewLabel->setStyleSheet(
        "QLabel {"
        "  color: #1f2937;"
        "  background: #f8fafc;"
        "  border: 1px solid #d1d5db;"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "}"
    );
    mainLayout->addWidget(m_sizePreviewLabel);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (!m_autoSizeCheckBox->isChecked()
            && m_minimumSizeSpin->value() > 0.0
            && m_maximumSizeSpin->value() > 0.0
            && m_minimumSizeSpin->value() > m_maximumSizeSpin->value()) {
            QMessageBox::warning(
                this,
                zh(u8"校验"),
                zh(u8"最小网格尺寸不能大于最大网格尺寸。")
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
    updateSizePreview();
}

void MeshSetupDialog::updateSizeControlState()
{
    const bool manual = !m_autoSizeCheckBox->isChecked();
    m_minimumSizeSpin->setEnabled(manual);
    m_maximumSizeSpin->setEnabled(manual);
}

void MeshSetupDialog::updateSizePreview()
{
    if (!m_sizePreviewLabel) {
        return;
    }

    const QString elementType = m_elementTypeCombo
        ? m_elementTypeCombo->currentText()
        : displayName(MeshElementType::Tetra4);
    if (m_autoSizeCheckBox->isChecked()) {
        m_sizePreviewLabel->setText(
            zh(u8"当前设置：%1，自动尺寸。Gmsh 会根据几何包围盒和曲率估算全局网格尺寸。")
                .arg(elementType)
        );
        return;
    }

    m_sizePreviewLabel->setText(
        zh(u8"当前设置：%1，手动尺寸。最小尺寸=%2，最大尺寸=%3。局部面组尺寸会在对应面组属性中覆盖全局尺寸。")
            .arg(elementType)
            .arg(m_minimumSizeSpin->value(), 0, 'g', 8)
            .arg(m_maximumSizeSpin->value(), 0, 'g', 8)
    );
}

void MeshSetupDialog::setElementType(MeshElementType elementType)
{
    const int index = m_elementTypeCombo->findData(toString(elementType));
    m_elementTypeCombo->setCurrentIndex(index >= 0 ? index : 0);
    updateSizePreview();
}

MeshElementType MeshSetupDialog::selectedElementType() const
{
    const QString value = m_elementTypeCombo->currentData().toString();
    if (value == toString(MeshElementType::Tetra10)) {
        return MeshElementType::Tetra10;
    }
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
