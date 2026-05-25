#include "ui/property/ResultPropertyView.h"

#include "result/ResultObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
QLabel *valueLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}
}

void ResultPropertyView::populate(QWidget *parent, const ResultObject &resultObject)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", valueLabel(resultObject.id, parent));
    form->addRow("Solver:", valueLabel(resultObject.solverName, parent));
    if (!resultObject.meshName.isEmpty()) {
        form->addRow("Mesh:", valueLabel(resultObject.meshName, parent));
    }
    form->addRow("Case Path:", valueLabel(resultObject.casePath, parent));
    form->addRow("Log File:", valueLabel(resultObject.logFile, parent));
    if (!resultObject.datFile.isEmpty()) {
        form->addRow("DAT File:", valueLabel(resultObject.datFile, parent));
    }
    if (!resultObject.frdFile.isEmpty()) {
        form->addRow("FRD File:", valueLabel(resultObject.frdFile, parent));
    }
    if (!resultObject.primaryFieldName.isEmpty()) {
        form->addRow("Primary Field:", valueLabel(resultObject.primaryFieldName, parent));
    }
    if (!resultObject.availableFields.isEmpty()) {
        form->addRow("Available Fields:", valueLabel(resultObject.availableFields.join(", "), parent));
    }
    form->addRow("Created At:", valueLabel(resultObject.createdAt, parent));
    form->addRow("Success:", valueLabel(resultObject.success ? "Yes" : "No", parent));
    form->addRow("Summary:", valueLabel(resultObject.summary, parent));
    dynamicLayout->addLayout(form);
}
