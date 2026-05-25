#include "ui/property/GeometryPropertyView.h"

#include "geometry/GeometryObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

void GeometryPropertyView::populate(QWidget *parent, const GeometryObject &geometry)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("JSON File:", new QLabel(geometry.jsonFile, parent));
    form->addRow("BREP File:", new QLabel(geometry.brepFile, parent));
    form->addRow("STEP File:", new QLabel(geometry.stepFile, parent));
    dynamicLayout->addLayout(form);
}
