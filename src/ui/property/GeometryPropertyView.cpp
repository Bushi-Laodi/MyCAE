#include "ui/property/GeometryPropertyView.h"

#include "geometry/GeometryObject.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString listText(const QStringList &values)
{
    return values.isEmpty() ? "-" : values.join("\n");
}

QLabel *valueLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QFormLayout *section(QVBoxLayout *layout, QWidget *parent, const QString &title)
{
    auto *group = new QGroupBox(title, parent);
    auto *form = new QFormLayout(group);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    layout->addWidget(group);
    return form;
}
}

void GeometryPropertyView::populate(
    QWidget *parent,
    const GeometryObject &geometry,
    const GeometryPropertyDetails &details
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    QFormLayout *shapeForm = section(dynamicLayout, parent, zh(u8"几何文件"));
    shapeForm->addRow(zh(u8"类型:"), valueLabel(geometry.type, parent));
    shapeForm->addRow(zh(u8"显示:"), valueLabel(details.visible ? zh(u8"显示") : zh(u8"隐藏"), parent));
    if (!details.center.isEmpty()) {
        shapeForm->addRow(zh(u8"中心:"), valueLabel(details.center, parent));
    }
    if (!details.dimensions.isEmpty()) {
        shapeForm->addRow(zh(u8"尺寸参数:"), valueLabel(details.dimensions, parent));
    }
    shapeForm->addRow(zh(u8"JSON 文件:"), valueLabel(geometry.jsonFile, parent));
    shapeForm->addRow(zh(u8"BREP 文件:"), valueLabel(details.brepFile.isEmpty() ? geometry.brepFile : details.brepFile, parent));
    shapeForm->addRow(zh(u8"STEP 文件:"), valueLabel(details.stepFile.isEmpty() ? geometry.stepFile : details.stepFile, parent));

    QFormLayout *dependencyForm = section(dynamicLayout, parent, zh(u8"关联状态"));
    dependencyForm->addRow(zh(u8"网格:"), valueLabel(listText(details.meshNames), parent));
    dependencyForm->addRow(zh(u8"过期网格:"), valueLabel(listText(details.staleMeshNames), parent));
    dependencyForm->addRow(zh(u8"面组:"), valueLabel(listText(details.faceGroupNames), parent));
    dependencyForm->addRow(zh(u8"需复查面组:"), valueLabel(listText(details.reviewFaceGroupNames), parent));
    dependencyForm->addRow(zh(u8"边界条件:"), valueLabel(listText(details.boundaryConditionNames), parent));
    dependencyForm->addRow(zh(u8"载荷:"), valueLabel(listText(details.loadNames), parent));
    dependencyForm->addRow(zh(u8"求解结果:"), valueLabel(listText(details.resultNames), parent));
    dependencyForm->addRow(zh(u8"过期结果:"), valueLabel(listText(details.staleResultNames), parent));
}
