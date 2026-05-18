#include "BoxDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QDoubleSpinBox *createDimensionSpinBox(QWidget *parent)
{
    auto *spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(0.001, 1000000.0);
    spinBox->setDecimals(3);
    spinBox->setValue(200.0);
    spinBox->setSingleStep(10.0);
    spinBox->setSuffix(" mm");
    return spinBox;
}
}

BoxDialog::BoxDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("创建长方体");
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    m_lengthSpinBox = createDimensionSpinBox(this);
    m_widthSpinBox = createDimensionSpinBox(this);
    m_heightSpinBox = createDimensionSpinBox(this);

    m_unitComboBox = new QComboBox(this);
    m_unitComboBox->addItems({"mm", "m"});
    connect(m_unitComboBox, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        const QString suffix = " " + unit;
        m_lengthSpinBox->setSuffix(suffix);
        m_widthSpinBox->setSuffix(suffix);
        m_heightSpinBox->setSuffix(suffix);
    });

    form->addRow("长度", m_lengthSpinBox);
    form->addRow("宽度", m_widthSpinBox);
    form->addRow("高度", m_heightSpinBox);
    form->addRow("单位", m_unitComboBox);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText("确定");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

BoxGeometry BoxDialog::boxParameters() const
{
    BoxGeometry box;
    box.length = m_lengthSpinBox->value();
    box.width = m_widthSpinBox->value();
    box.height = m_heightSpinBox->value();
    box.unit = m_unitComboBox->currentText();
    return box;
}
