#include "SphereDialog.h"

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

QDoubleSpinBox *createRadiusSpinBox(QWidget *parent)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(0.001, 1000000.0);
    spinBox->setDecimals(3);
    spinBox->setValue(50.0);
    spinBox->setSingleStep(10.0);
    spinBox->setSuffix(" mm");
    return spinBox;
}

QDoubleSpinBox *createCoordinateSpinBox(QWidget *parent)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(-1000000.0, 1000000.0);
    spinBox->setDecimals(3);
    spinBox->setValue(0.0);
    spinBox->setSingleStep(10.0);
    spinBox->setSuffix(" mm");
    return spinBox;
}
}

SphereDialog::SphereDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"创建球体"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_centerXSpinBox = createCoordinateSpinBox(this);
    m_centerYSpinBox = createCoordinateSpinBox(this);
    m_centerZSpinBox = createCoordinateSpinBox(this);
    m_radiusSpinBox = createRadiusSpinBox(this);

    m_unitComboBox = new QComboBox(this);
    m_unitComboBox->addItems({"mm", "m"});
    connect(m_unitComboBox, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        const QString suffix = " " + unit;
        m_centerXSpinBox->setSuffix(suffix);
        m_centerYSpinBox->setSuffix(suffix);
        m_centerZSpinBox->setSuffix(suffix);
        m_radiusSpinBox->setSuffix(suffix);
    });

    form->addRow(zh(u8"中心 X"), m_centerXSpinBox);
    form->addRow(zh(u8"中心 Y"), m_centerYSpinBox);
    form->addRow(zh(u8"中心 Z"), m_centerZSpinBox);
    form->addRow(zh(u8"半径"), m_radiusSpinBox);
    form->addRow(zh(u8"单位"), m_unitComboBox);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

SphereGeometry SphereDialog::sphereParameters() const
{
    SphereGeometry sphere;
    sphere.centerX = m_centerXSpinBox->value();
    sphere.centerY = m_centerYSpinBox->value();
    sphere.centerZ = m_centerZSpinBox->value();
    sphere.radius = m_radiusSpinBox->value();
    sphere.unit = m_unitComboBox->currentText();
    return sphere;
}
