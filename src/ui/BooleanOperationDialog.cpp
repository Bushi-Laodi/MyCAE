#include "BooleanOperationDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

BooleanOperationDialog::BooleanOperationDialog(
    const QVector<GeometryObject> &geometries,
    const QString &preferredLeftGeometryName,
    QWidget *parent
)
    : QDialog(parent)
{
    setWindowTitle("Boolean Operation");
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
    m_operationCombo->addItem("Union", static_cast<int>(GeometryBooleanOperationType::Union));
    m_operationCombo->addItem("Cut", static_cast<int>(GeometryBooleanOperationType::Cut));
    m_operationCombo->addItem("Common", static_cast<int>(GeometryBooleanOperationType::Common));

    m_resultNameEdit = new QLineEdit(this);
    m_resultNameEdit->setPlaceholderText("Leave empty to generate a name automatically");

    form->addRow("Left Geometry:", m_leftGeometryCombo);
    form->addRow("Right Geometry:", m_rightGeometryCombo);
    form->addRow("Operation:", m_operationCombo);
    form->addRow("Result Name:", m_resultNameEdit);
    mainLayout->addLayout(form);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
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
        QMessageBox::warning(this, "Validation", "Please choose two geometry objects.");
        return false;
    }
    if (m_leftGeometryCombo->currentText() == m_rightGeometryCombo->currentText()) {
        QMessageBox::warning(this, "Validation", "Boolean inputs must be different geometry objects.");
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
