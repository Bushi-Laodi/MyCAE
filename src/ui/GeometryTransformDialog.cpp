#include "ui/GeometryTransformDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QDoubleSpinBox *coordinateSpin(QWidget *parent, double value = 0.0)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(-1000000000.0, 1000000000.0);
    spin->setDecimals(6);
    spin->setSingleStep(1.0);
    spin->setValue(value);
    spin->setSuffix(" mm");
    return spin;
}

QDoubleSpinBox *angleSpin(QWidget *parent)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(-3600.0, 3600.0);
    spin->setDecimals(3);
    spin->setSingleStep(5.0);
    spin->setSuffix(" deg");
    return spin;
}

QDoubleSpinBox *scaleSpin(QWidget *parent)
{
    auto *spin = new QDoubleSpinBox(parent);
    spin->setRange(0.000001, 1000000.0);
    spin->setDecimals(6);
    spin->setSingleStep(0.1);
    spin->setValue(1.0);
    return spin;
}
}

GeometryTransformDialog::GeometryTransformDialog(const GeometryCenter &currentCenter, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"变换选中几何体"));
    setModal(true);
    setupUi(currentCenter);
}

GeometryTransformParameters GeometryTransformDialog::parameters() const
{
    GeometryTransformParameters parameters;
    parameters.setAbsoluteCenter = m_absoluteCenterCheck->isChecked();
    parameters.targetCenterX = m_targetCenterX->value();
    parameters.targetCenterY = m_targetCenterY->value();
    parameters.targetCenterZ = m_targetCenterZ->value();
    parameters.translateX = m_translateX->value();
    parameters.translateY = m_translateY->value();
    parameters.translateZ = m_translateZ->value();
    parameters.rotateXDegrees = m_rotateX->value();
    parameters.rotateYDegrees = m_rotateY->value();
    parameters.rotateZDegrees = m_rotateZ->value();
    parameters.uniformScale = m_uniformScale->value();
    return parameters;
}

void GeometryTransformDialog::setupUi(const GeometryCenter &currentCenter)
{
    auto *layout = new QVBoxLayout(this);

    auto *centerGroup = new QGroupBox(zh(u8"精确中心"), this);
    auto *centerForm = new QFormLayout(centerGroup);
    m_absoluteCenterCheck = new QCheckBox(zh(u8"设置为绝对中心坐标"), centerGroup);
    m_targetCenterX = coordinateSpin(centerGroup, currentCenter.x);
    m_targetCenterY = coordinateSpin(centerGroup, currentCenter.y);
    m_targetCenterZ = coordinateSpin(centerGroup, currentCenter.z);
    centerForm->addRow(m_absoluteCenterCheck);
    centerForm->addRow("X:", m_targetCenterX);
    centerForm->addRow("Y:", m_targetCenterY);
    centerForm->addRow("Z:", m_targetCenterZ);
    layout->addWidget(centerGroup);

    auto *translateGroup = new QGroupBox(zh(u8"增量平移"), this);
    auto *translateForm = new QFormLayout(translateGroup);
    m_translateX = coordinateSpin(translateGroup);
    m_translateY = coordinateSpin(translateGroup);
    m_translateZ = coordinateSpin(translateGroup);
    translateForm->addRow("dX:", m_translateX);
    translateForm->addRow("dY:", m_translateY);
    translateForm->addRow("dZ:", m_translateZ);
    layout->addWidget(translateGroup);

    auto *rotateGroup = new QGroupBox(zh(u8"绕当前中心旋转"), this);
    auto *rotateForm = new QFormLayout(rotateGroup);
    m_rotateX = angleSpin(rotateGroup);
    m_rotateY = angleSpin(rotateGroup);
    m_rotateZ = angleSpin(rotateGroup);
    rotateForm->addRow("X:", m_rotateX);
    rotateForm->addRow("Y:", m_rotateY);
    rotateForm->addRow("Z:", m_rotateZ);
    layout->addWidget(rotateGroup);

    auto *scaleGroup = new QGroupBox(zh(u8"统一缩放"), this);
    auto *scaleForm = new QFormLayout(scaleGroup);
    m_uniformScale = scaleSpin(scaleGroup);
    scaleForm->addRow(zh(u8"比例:"), m_uniformScale);
    layout->addWidget(scaleGroup);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(m_absoluteCenterCheck, &QCheckBox::toggled, this, [this]() {
        updateAbsoluteCenterState();
    });
    updateAbsoluteCenterState();
}

void GeometryTransformDialog::updateAbsoluteCenterState()
{
    const bool enabled = m_absoluteCenterCheck->isChecked();
    m_targetCenterX->setEnabled(enabled);
    m_targetCenterY->setEnabled(enabled);
    m_targetCenterZ->setEnabled(enabled);
}
