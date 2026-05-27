#include "BooleanOperationDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

BooleanOperationDialog::BooleanOperationDialog(
    const QVector<GeometryObject> &geometries,
    const QString &preferredLeftGeometryName,
    QWidget *parent
)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"布尔运算"));
    setupUi(geometries, preferredLeftGeometryName);
}

void BooleanOperationDialog::setupUi(
    const QVector<GeometryObject> &geometries,
    const QString &preferredLeftGeometryName
)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_leftGeometryCombo = new QComboBox(this);
    m_rightGeometryCombo = new QComboBox(this);
    for (const GeometryObject &geometry : geometries) {
        m_leftGeometryCombo->addItem(geometry.name);
        m_rightGeometryCombo->addItem(geometry.name);
    }
    setComboText(m_leftGeometryCombo, preferredLeftGeometryName);
    if (m_rightGeometryCombo->count() > 1 && m_rightGeometryCombo->currentText() == m_leftGeometryCombo->currentText()) {
        m_rightGeometryCombo->setCurrentIndex((m_leftGeometryCombo->currentIndex() + 1) % m_rightGeometryCombo->count());
    }

    m_operationCombo = new QComboBox(this);
    m_operationCombo->addItem(zh(u8"并集"), static_cast<int>(GeometryBooleanOperationType::Union));
    m_operationCombo->addItem(zh(u8"切除"), static_cast<int>(GeometryBooleanOperationType::Cut));
    m_operationCombo->addItem(zh(u8"交集"), static_cast<int>(GeometryBooleanOperationType::Common));

    m_resultNameEdit = new QLineEdit(this);
    m_resultNameEdit->setPlaceholderText(zh(u8"留空则自动生成名称"));

    form->addRow(zh(u8"左侧几何:"), m_leftGeometryCombo);
    form->addRow(zh(u8"右侧几何:"), m_rightGeometryCombo);
    form->addRow(zh(u8"运算:"), m_operationCombo);
    form->addRow(zh(u8"结果名称:"), m_resultNameEdit);
    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (validate()) {
            accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

BooleanOperationDialogResult BooleanOperationDialog::operation() const
{
    BooleanOperationDialogResult result;
    result.leftGeometryName = m_leftGeometryCombo->currentText().trimmed();
    result.rightGeometryName = m_rightGeometryCombo->currentText().trimmed();
    result.operationType = static_cast<GeometryBooleanOperationType>(m_operationCombo->currentData().toInt());
    result.resultName = m_resultNameEdit->text().trimmed();
    return result;
}

std::optional<BooleanOperationDialogResult> BooleanOperationDialog::getOperation(
    QWidget *parent,
    const QVector<GeometryObject> &geometries,
    const QString &preferredLeftGeometryName
)
{
    BooleanOperationDialog dialog(geometries, preferredLeftGeometryName, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.operation();
    }
    return std::nullopt;
}

bool BooleanOperationDialog::validate()
{
    if (m_leftGeometryCombo->currentText().trimmed().isEmpty()
            || m_rightGeometryCombo->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, zh(u8"校验"), zh(u8"请选择两个几何对象。"));
        return false;
    }
    if (m_leftGeometryCombo->currentText() == m_rightGeometryCombo->currentText()) {
        QMessageBox::warning(this, zh(u8"校验"), zh(u8"布尔运算的输入必须是不同几何对象。"));
        return false;
    }
    return true;
}

void BooleanOperationDialog::setComboText(QComboBox *combo, const QString &text)
{
    const int index = combo->findText(text);
    if (index >= 0) {
        combo->setCurrentIndex(index);
    }
}
