#include "ui/property/ResultPropertyView.h"

#include "result/ResultObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString yesNoText(bool value)
{
    return value ? zh(u8"是") : zh(u8"否");
}

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
    form->addRow(zh(u8"状态:"), valueLabel(resultObject.stale ? zh(u8"已过期，需要重新求解") : zh(u8"有效"), parent));
    if (resultObject.stale && !resultObject.staleReason.isEmpty()) {
        form->addRow(zh(u8"过期原因:"), valueLabel(resultObject.staleReason, parent));
    }
    form->addRow(zh(u8"求解器:"), valueLabel(resultObject.solverName, parent));
    if (!resultObject.meshName.isEmpty()) {
        form->addRow(zh(u8"网格:"), valueLabel(resultObject.meshName, parent));
    }
    form->addRow(zh(u8"算例路径:"), valueLabel(resultObject.casePath, parent));
    form->addRow(zh(u8"日志文件:"), valueLabel(resultObject.logFile, parent));
    if (!resultObject.datFile.isEmpty()) {
        form->addRow("DAT File:", valueLabel(resultObject.datFile, parent));
    }
    if (!resultObject.frdFile.isEmpty()) {
        form->addRow("FRD File:", valueLabel(resultObject.frdFile, parent));
    }
    if (!resultObject.primaryFieldName.isEmpty()) {
        form->addRow(zh(u8"主结果场:"), valueLabel(resultObject.primaryFieldName, parent));
    }
    if (!resultObject.displayFieldName.isEmpty()) {
        form->addRow(zh(u8"显示结果场:"), valueLabel(resultObject.displayFieldName, parent));
    }
    form->addRow(zh(u8"变形比例:"), valueLabel(QString::number(resultObject.deformationScale, 'g', 6), parent));
    if (!resultObject.availableFields.isEmpty()) {
        form->addRow(zh(u8"可用结果场:"), valueLabel(resultObject.availableFields.join(", "), parent));
    }
    form->addRow(zh(u8"标量范围:"), valueLabel(QString("%1 - %2")
        .arg(resultObject.scalarMin, 0, 'g', 6)
        .arg(resultObject.scalarMax, 0, 'g', 6), parent));
    form->addRow(zh(u8"节点覆盖:"), valueLabel(QString("%1 / %2")
        .arg(resultObject.matchedNodeCount)
        .arg(resultObject.meshNodeCount), parent));
    form->addRow(zh(u8"单元覆盖:"), valueLabel(QString("%1 / %2")
        .arg(resultObject.matchedElementCount)
        .arg(resultObject.meshElementCount), parent));
    form->addRow(zh(u8"结果文件:"), valueLabel(resultObject.resultFilesComplete ? zh(u8"完整") : zh(u8"不完整"), parent));
    if (!resultObject.checkMessages.isEmpty()) {
        form->addRow(zh(u8"检查:"), valueLabel(resultObject.checkMessages.join("; "), parent));
    }
    form->addRow(zh(u8"创建时间:"), valueLabel(resultObject.createdAt, parent));
    form->addRow(zh(u8"是否成功:"), valueLabel(yesNoText(resultObject.success), parent));
    form->addRow(zh(u8"摘要:"), valueLabel(resultObject.summary, parent));
    dynamicLayout->addLayout(form);
}
