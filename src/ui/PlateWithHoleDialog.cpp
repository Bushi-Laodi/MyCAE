#include "ui/PlateWithHoleDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QDoubleSpinBox *positiveSpinBox(QWidget *parent, double value)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(0.001, 1.0e9);
    spinBox->setDecimals(3);
    spinBox->setSingleStep(10.0);
    spinBox->setValue(value);
    spinBox->setSuffix(" mm");
    return spinBox;
}

QDoubleSpinBox *coordinateSpinBox(QWidget *parent)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(-1.0e9, 1.0e9);
    spinBox->setDecimals(3);
    spinBox->setSingleStep(10.0);
    spinBox->setSuffix(" mm");
    return spinBox;
}
}

PlateWithHoleDialog::PlateWithHoleDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"创建带孔板"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_lengthSpinBox = positiveSpinBox(this, 200.0);
    m_widthSpinBox = positiveSpinBox(this, 100.0);
    m_thicknessSpinBox = positiveSpinBox(this, 10.0);
    m_holeRadiusSpinBox = positiveSpinBox(this, 20.0);
    m_centerXSpinBox = coordinateSpinBox(this);
    m_centerYSpinBox = coordinateSpinBox(this);
    m_centerZSpinBox = coordinateSpinBox(this);

    m_unitComboBox = new QComboBox(this);
    m_unitComboBox->addItems({"mm", "m"});
    connect(m_unitComboBox, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        updateSuffixes(unit);
    });

    form->addRow(zh(u8"长度"), m_lengthSpinBox);
    form->addRow(zh(u8"宽度"), m_widthSpinBox);
    form->addRow(zh(u8"厚度"), m_thicknessSpinBox);
    form->addRow(zh(u8"孔半径"), m_holeRadiusSpinBox);
    form->addRow(zh(u8"中心 X"), m_centerXSpinBox);
    form->addRow(zh(u8"中心 Y"), m_centerYSpinBox);
    form->addRow(zh(u8"中心 Z"), m_centerZSpinBox);
    form->addRow(zh(u8"单位"), m_unitComboBox);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (validate()) {
            accept();
        }
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

PlateWithHoleGeometry PlateWithHoleDialog::plateParameters() const
{
    PlateWithHoleGeometry plate;
    plate.length = m_lengthSpinBox->value();
    plate.width = m_widthSpinBox->value();
    plate.thickness = m_thicknessSpinBox->value();
    plate.holeRadius = m_holeRadiusSpinBox->value();
    plate.centerX = m_centerXSpinBox->value();
    plate.centerY = m_centerYSpinBox->value();
    plate.centerZ = m_centerZSpinBox->value();
    plate.unit = m_unitComboBox->currentText();
    return plate;
}

bool PlateWithHoleDialog::validate()
{
    const double limit = std::min(m_lengthSpinBox->value(), m_widthSpinBox->value()) * 0.5;
    if (m_holeRadiusSpinBox->value() >= limit) {
        QMessageBox::warning(
            this,
            zh(u8"参数校验"),
            zh(u8"孔半径必须小于长度和宽度较小值的一半。")
        );
        return false;
    }
    return true;
}

void PlateWithHoleDialog::updateSuffixes(const QString &unit)
{
    const QString suffix = " " + unit;
    for (QDoubleSpinBox *spinBox : {
             m_lengthSpinBox,
             m_widthSpinBox,
             m_thicknessSpinBox,
             m_holeRadiusSpinBox,
             m_centerXSpinBox,
             m_centerYSpinBox,
             m_centerZSpinBox
         }) {
        spinBox->setSuffix(suffix);
    }
}
