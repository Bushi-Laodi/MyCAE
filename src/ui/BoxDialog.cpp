#include "BoxDialog.h"

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

BoxDialog::BoxDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"创建长方体"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_centerXSpinBox = createCoordinateSpinBox(this);
    m_centerYSpinBox = createCoordinateSpinBox(this);
    m_centerZSpinBox = createCoordinateSpinBox(this);
    m_lengthSpinBox = createDimensionSpinBox(this, 200.0);
    m_widthSpinBox = createDimensionSpinBox(this, 200.0);
    m_heightSpinBox = createDimensionSpinBox(this, 200.0);

    m_unitComboBox = new QComboBox(this);
    m_unitComboBox->addItems({"mm", "m"});
    connect(m_unitComboBox, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        const QString suffix = " " + unit;
        m_centerXSpinBox->setSuffix(suffix);
        m_centerYSpinBox->setSuffix(suffix);
        m_centerZSpinBox->setSuffix(suffix);
        m_lengthSpinBox->setSuffix(suffix);
        m_widthSpinBox->setSuffix(suffix);
        m_heightSpinBox->setSuffix(suffix);
    });

    form->addRow(zh(u8"中心 X"), m_centerXSpinBox);
    form->addRow(zh(u8"中心 Y"), m_centerYSpinBox);
    form->addRow(zh(u8"中心 Z"), m_centerZSpinBox);
    form->addRow(zh(u8"长度"), m_lengthSpinBox);
    form->addRow(zh(u8"宽度"), m_widthSpinBox);
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

BoxGeometry BoxDialog::boxParameters() const
{
    BoxGeometry box;
    box.centerX = m_centerXSpinBox->value();
    box.centerY = m_centerYSpinBox->value();
    box.centerZ = m_centerZSpinBox->value();
    box.length = m_lengthSpinBox->value();
    box.width = m_widthSpinBox->value();
    box.height = m_heightSpinBox->value();
    box.unit = m_unitComboBox->currentText();
    return box;
}
