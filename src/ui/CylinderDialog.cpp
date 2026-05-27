#include "CylinderDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QDoubleSpinBox *createDimensionSpinBox(QWidget *parent, double value)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(0.001, 1000000.0);
    spinBox->setDecimals(3);
    spinBox->setValue(value);
    spinBox->setSingleStep(10.0);
    spinBox->setSuffix(" mm");
    return spinBox;
}
}

CylinderDialog::CylinderDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"创建圆柱体"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_radiusSpinBox = createDimensionSpinBox(this, 50.0);
    m_heightSpinBox = createDimensionSpinBox(this, 200.0);

    m_unitComboBox = new QComboBox(this);
    m_unitComboBox->addItems({"mm", "m"});
    connect(m_unitComboBox, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        const QString suffix = " " + unit;
        m_radiusSpinBox->setSuffix(suffix);
        m_heightSpinBox->setSuffix(suffix);
    });

    form->addRow(zh(u8"半径"), m_radiusSpinBox);
    form->addRow(zh(u8"高度"), m_heightSpinBox);
    form->addRow(zh(u8"单位"), m_unitComboBox);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

CylinderGeometry CylinderDialog::cylinderParameters() const
{
    CylinderGeometry cylinder;
    cylinder.radius = m_radiusSpinBox->value();
    cylinder.height = m_heightSpinBox->value();
    cylinder.unit = m_unitComboBox->currentText();
    return cylinder;
}
