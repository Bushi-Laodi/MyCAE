#include "ui/property/PickPropertyView.h"

#include <QFormLayout>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

void PickPropertyView::populate(
    QWidget *parent,
    PickMode mode,
    const QString &geometryName,
    const std::vector<int> &faceIndices
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;

    QStringList faceIndexTexts;
    for (const int faceIndex : faceIndices) {
        faceIndexTexts.append(QString::number(faceIndex));
    }

    form->addRow("Pick Mode:", new QLabel(pickModeName(mode), parent));
    form->addRow("Geometry:", new QLabel(geometryName, parent));
    form->addRow("Selected Faces:", new QLabel(QString::number(faceIndices.size()), parent));
    form->addRow("Face Indices:", new QLabel(faceIndexTexts.join(", "), parent));
    dynamicLayout->addLayout(form);
}
