#include "ui/property/FaceGroupPropertyView.h"

#include "geometry/FaceGroup.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

void FaceGroupPropertyView::populate(QWidget *parent, const FaceGroup &faceGroup)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(faceGroup.id, parent));
    form->addRow("Name:", new QLabel(faceGroup.name, parent));
    form->addRow("Geometry:", new QLabel(faceGroup.geometryName, parent));
    form->addRow("Role:", new QLabel(faceGroup.role, parent));
    form->addRow("Face Count:", new QLabel(QString::number(faceGroup.faceIndices.size()), parent));
    form->addRow("Physical Group:", new QLabel(faceGroup.physicalGroupEnabled ? "Enabled" : "Disabled", parent));
    form->addRow("Local Mesh:", new QLabel(faceGroup.localMeshEnabled ? "Enabled" : "Disabled", parent));
    form->addRow("Local Mesh Size:", new QLabel(QString::number(faceGroup.localMeshSize), parent));

    QStringList faceIndexTexts;
    for (const int faceIndex : faceGroup.faceIndices) {
        faceIndexTexts.append(QString::number(faceIndex));
    }
    form->addRow("Face Indices:", new QLabel(faceIndexTexts.join(", "), parent));

    if (!faceGroup.faceReferences.empty()) {
        const FaceReference &firstReference = faceGroup.faceReferences.front();
        form->addRow("First Face Area:", new QLabel(QString::number(firstReference.area), parent));
        form->addRow(
            "First Face Normal:",
            new QLabel(
                QString("(%1, %2, %3)")
                    .arg(firstReference.normal.x)
                    .arg(firstReference.normal.y)
                    .arg(firstReference.normal.z),
                parent
            )
        );
    }
    dynamicLayout->addLayout(form);
}
