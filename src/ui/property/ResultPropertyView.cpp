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
    form->addRow("Case Path:", valueLabel(resultObject.casePath, parent));
    form->addRow("Log File:", valueLabel(resultObject.logFile, parent));
    form->addRow("Created At:", valueLabel(resultObject.createdAt, parent));
    form->addRow("Success:", valueLabel(resultObject.success ? "Yes" : "No", parent));
    form->addRow("Summary:", valueLabel(resultObject.summary, parent));
    dynamicLayout->addLayout(form);
}
