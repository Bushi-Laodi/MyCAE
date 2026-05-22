#include "PropertyPanel.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "mesh/MeshObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *form = new QFormLayout(this);
    m_selectionValue = new QLabel("None", this);
    m_typeValue = new QLabel("-", this);
    m_nameValue = new QLabel("-", this);
    m_radiusValue = new QLabel("-", this);
    m_lengthValue = new QLabel("-", this);
    m_widthValue = new QLabel("-", this);
    m_heightValue = new QLabel("-", this);
    m_sourceGeometryValue = new QLabel("-", this);
    m_sourceGeometryTypeValue = new QLabel("-", this);
    m_sourceStepFileValue = new QLabel("-", this);
    m_meshFileValue = new QLabel("-", this);
    m_nodeCountValue = new QLabel("-", this);
    m_tetraCountValue = new QLabel("-", this);
    m_createdAtValue = new QLabel("-", this);

    form->addRow("Selection", m_selectionValue);
    form->addRow("Type", m_typeValue);
    form->addRow("Name", m_nameValue);
    form->addRow("Radius", m_radiusValue);
    form->addRow("Length", m_lengthValue);
    form->addRow("Width", m_widthValue);
    form->addRow("Height", m_heightValue);
    form->addRow("Source Geometry", m_sourceGeometryValue);
    form->addRow("Source Type", m_sourceGeometryTypeValue);
    form->addRow("Source STEP", m_sourceStepFileValue);
    form->addRow("Mesh File", m_meshFileValue);
    form->addRow("Node Count", m_nodeCountValue);
    form->addRow("Tetra Count", m_tetraCountValue);
    form->addRow("Created At", m_createdAtValue);
}

void PropertyPanel::showEmptySelection()
{
    m_selectionValue->setText("None");
    m_typeValue->setText("-");
    m_nameValue->setText("-");
    m_radiusValue->setText("-");
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_sourceGeometryValue->setText("-");
    m_sourceGeometryTypeValue->setText("-");
    m_sourceStepFileValue->setText("-");
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");
}

void PropertyPanel::showBoxGeometry(const BoxGeometry &box)
{
    const QString suffix = " " + box.unit;
    m_selectionValue->setText(box.name);
    m_typeValue->setText("Box");
    m_nameValue->setText(box.name);
    m_radiusValue->setText("-");
    m_lengthValue->setText(QString::number(box.length) + suffix);
    m_widthValue->setText(QString::number(box.width) + suffix);
    m_heightValue->setText(QString::number(box.height) + suffix);
    m_sourceGeometryValue->setText("-");
    m_sourceGeometryTypeValue->setText("-");
    m_sourceStepFileValue->setText(box.occStepFile);
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");
}

void PropertyPanel::showCylinderGeometry(const CylinderGeometry &cylinder)
{
    const QString suffix = " " + cylinder.unit;
    m_selectionValue->setText(cylinder.name);
    m_typeValue->setText("Cylinder");
    m_nameValue->setText(cylinder.name);
    m_radiusValue->setText(QString::number(cylinder.radius) + suffix);
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText(QString::number(cylinder.height) + suffix);
    m_sourceGeometryValue->setText("-");
    m_sourceGeometryTypeValue->setText("-");
    m_sourceStepFileValue->setText(cylinder.occStepFile);
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");
}

void PropertyPanel::showMeshObject(const MeshObject &meshObject)
{
    m_selectionValue->setText(meshObject.name);
    m_typeValue->setText(meshObject.type);
    m_nameValue->setText(meshObject.name);
    m_radiusValue->setText("-");
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_sourceGeometryValue->setText(meshObject.sourceGeometryName);
    m_sourceGeometryTypeValue->setText(meshObject.sourceGeometryType);
    m_sourceStepFileValue->setText(meshObject.sourceStepFile);
    m_meshFileValue->setText(meshObject.mshFile);
    m_nodeCountValue->setText(QString::number(meshObject.nodeCount));
    m_tetraCountValue->setText(QString::number(meshObject.tetraCount));
    m_createdAtValue->setText(meshObject.createdAt);
}
