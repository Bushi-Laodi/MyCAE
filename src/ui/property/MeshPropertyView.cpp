#include "ui/property/MeshPropertyView.h"

#include "mesh/MeshObject.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString numberText(double value)
{
    return QString::number(value, 'g', 8);
}

QLabel *valueLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QFormLayout *addSection(QVBoxLayout *layout, QWidget *parent, const QString &title)
{
    auto *group = new QGroupBox(title, parent);
    auto *form = new QFormLayout(group);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    layout->addWidget(group);
    return form;
}

QString globalSizeText(const MeshObject &meshObject)
{
    if (meshObject.meshAutoSize) {
        return zh(u8"自动，Gmsh 根据几何尺度估算");
    }
    return zh(u8"手动，min=%1，max=%2")
        .arg(numberText(meshObject.meshMinimumSize))
        .arg(numberText(meshObject.meshMaximumSize));
}

QString localControlText(const MeshObject &meshObject)
{
    return meshObject.localMeshControls.isEmpty()
        ? zh(u8"无局部尺寸控制")
        : meshObject.localMeshControls.join("\n");
}

QString warningsText(const MeshObject &meshObject)
{
    return meshObject.qualityWarnings.isEmpty()
        ? zh(u8"无")
        : meshObject.qualityWarnings.join("\n");
}

QString qualityStatusText(const MeshObject &meshObject)
{
    if (!meshObject.qualityChecked) {
        return zh(u8"未检查");
    }
    return meshObject.qualityStatus.trimmed().isEmpty() ? zh(u8"已检查") : meshObject.qualityStatus;
}
}

void MeshPropertyView::populate(QWidget *parent, const MeshObject &meshObject)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    QFormLayout *sizeForm = addSection(dynamicLayout, parent, zh(u8"尺寸参数"));
    sizeForm->addRow(zh(u8"状态:"), valueLabel(meshObject.stale ? zh(u8"已过期，需要重新生成") : zh(u8"有效"), parent));
    if (meshObject.stale && !meshObject.staleReason.isEmpty()) {
        sizeForm->addRow(zh(u8"过期原因:"), valueLabel(meshObject.staleReason, parent));
    }
    sizeForm->addRow(zh(u8"单元类型:"), valueLabel(meshObject.type, parent));
    sizeForm->addRow(zh(u8"全局尺寸:"), valueLabel(globalSizeText(meshObject), parent));
    sizeForm->addRow(zh(u8"局部尺寸:"), valueLabel(localControlText(meshObject), parent));

    QFormLayout *qualityForm = addSection(dynamicLayout, parent, zh(u8"质量检查"));
    qualityForm->addRow(zh(u8"状态:"), valueLabel(qualityStatusText(meshObject), parent));
    qualityForm->addRow(zh(u8"最小边长:"), valueLabel(numberText(meshObject.minimumEdgeLength), parent));
    qualityForm->addRow(zh(u8"最大边长:"), valueLabel(numberText(meshObject.maximumEdgeLength), parent));
    qualityForm->addRow(zh(u8"平均边长:"), valueLabel(numberText(meshObject.averageEdgeLength), parent));
    qualityForm->addRow(zh(u8"最小体积:"), valueLabel(numberText(meshObject.minimumTetraVolume), parent));
    qualityForm->addRow(zh(u8"最大体积:"), valueLabel(numberText(meshObject.maximumTetraVolume), parent));
    qualityForm->addRow(zh(u8"平均体积:"), valueLabel(numberText(meshObject.averageTetraVolume), parent));
    qualityForm->addRow(zh(u8"最大长宽比:"), valueLabel(numberText(meshObject.maximumAspectRatio), parent));
    qualityForm->addRow(zh(u8"平均长宽比:"), valueLabel(numberText(meshObject.averageAspectRatio), parent));
    qualityForm->addRow(zh(u8"异常四面体:"), valueLabel(QString::number(meshObject.invalidTetraCount), parent));
    qualityForm->addRow(zh(u8"近零体积:"), valueLabel(QString::number(meshObject.degenerateTetraCount), parent));
    qualityForm->addRow(zh(u8"高长宽比:"), valueLabel(QString::number(meshObject.highAspectRatioTetraCount), parent));
    qualityForm->addRow(zh(u8"提示:"), valueLabel(warningsText(meshObject), parent));
}
