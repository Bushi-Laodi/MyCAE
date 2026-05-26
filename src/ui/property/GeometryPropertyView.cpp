#include "ui/property/GeometryPropertyView.h"

#include "geometry/GeometryObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

void GeometryPropertyView::populate(QWidget *parent, const GeometryObject &geometry)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow(zh(u8"JSON 文件:"), new QLabel(geometry.jsonFile, parent));
    form->addRow(zh(u8"BREP 文件:"), new QLabel(geometry.brepFile, parent));
    form->addRow(zh(u8"STEP 文件:"), new QLabel(geometry.stepFile, parent));
    dynamicLayout->addLayout(form);
}
