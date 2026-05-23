#include "PropertyPanel.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;
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

    m_mainLayout->addLayout(form);

    // Dynamic area for solver data (materials, BCs, loads)
    m_dynamicArea = new QWidget(this);
    m_mainLayout->addWidget(m_dynamicArea);
    m_mainLayout->addStretch();
}

void PropertyPanel::clearAll()
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

QWidget *PropertyPanel::resetDynamicArea()
{
    if (m_dynamicArea) {
        m_mainLayout->removeWidget(m_dynamicArea);
        m_dynamicArea->deleteLater();
    }

    m_dynamicArea = new QWidget(this);
    m_mainLayout->insertWidget(1, m_dynamicArea);
    return m_dynamicArea;
}

void PropertyPanel::showEmptySelection()
{
    clearAll();
    resetDynamicArea();
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

    resetDynamicArea();
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

    resetDynamicArea();
}

void PropertyPanel::showGeometryObject(const GeometryObject &geometry)
{
    clearAll();
    m_selectionValue->setText(geometry.name);
    m_typeValue->setText(geometry.type);
    m_nameValue->setText(geometry.name);
    m_sourceStepFileValue->setText(geometry.stepFile);

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);
    auto *form = new QFormLayout;
    form->addRow("JSON File:", new QLabel(geometry.jsonFile, m_dynamicArea));
    form->addRow("BREP File:", new QLabel(geometry.brepFile, m_dynamicArea));
    form->addRow("STEP File:", new QLabel(geometry.stepFile, m_dynamicArea));
    dynamicLayout->addLayout(form);
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

    resetDynamicArea();
}

void PropertyPanel::showFaceGroup(const FaceGroup &faceGroup)
{
    clearAll();
    m_selectionValue->setText(faceGroup.id);
    m_typeValue->setText("Face Group");
    m_nameValue->setText(faceGroup.name);
    m_sourceGeometryValue->setText(faceGroup.geometryName);

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(faceGroup.id, m_dynamicArea));
    form->addRow("Name:", new QLabel(faceGroup.name, m_dynamicArea));
    form->addRow("Geometry:", new QLabel(faceGroup.geometryName, m_dynamicArea));
    form->addRow("Role:", new QLabel(faceGroup.role, m_dynamicArea));
    dynamicLayout->addLayout(form);
}

void PropertyPanel::showMaterialCategory(const std::vector<Material> &materials)
{
    clearAll();
    m_selectionValue->setText("Materials");

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);

    if (materials.empty()) {
        dynamicLayout->addWidget(new QLabel("No materials defined.", m_dynamicArea));
    } else {
        for (size_t i = 0; i < materials.size(); ++i) {
            const Material &mat = materials[i];
            auto *groupLabel = new QLabel(
                QString("<b>%1. %2</b>").arg(i + 1).arg(mat.name), m_dynamicArea);
            dynamicLayout->addWidget(groupLabel);

            auto *form = new QFormLayout;
            form->addRow("ID:", new QLabel(mat.id, m_dynamicArea));
            form->addRow("Domain:", new QLabel(toString(mat.domain), m_dynamicArea));
            form->addRow("Viscosity Model:", new QLabel(toString(mat.viscosityModel), m_dynamicArea));

            if (mat.hasDensity) {
                form->addRow("Density:", new QLabel(
                    QString::number(mat.density) + " " + mat.densityUnit, m_dynamicArea));
            }
            if (mat.hasDynamicViscosity) {
                form->addRow("Dynamic Viscosity:", new QLabel(
                    QString::number(mat.dynamicViscosity) + " " + mat.dynamicViscosityUnit, m_dynamicArea));
            }
            if (mat.hasKinematicViscosity) {
                form->addRow("Kinematic Viscosity:", new QLabel(
                    QString::number(mat.kinematicViscosity) + " " + mat.kinematicViscosityUnit, m_dynamicArea));
            }

            dynamicLayout->addLayout(form);
        }
    }
}

void PropertyPanel::showBoundaryConditionCategory(const std::vector<BoundaryCondition> &boundaryConditions)
{
    clearAll();
    m_selectionValue->setText("Boundary Conditions");

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);

    if (boundaryConditions.empty()) {
        dynamicLayout->addWidget(new QLabel("No boundary conditions defined.", m_dynamicArea));
    } else {
        for (size_t i = 0; i < boundaryConditions.size(); ++i) {
            const BoundaryCondition &bc = boundaryConditions[i];
            auto *groupLabel = new QLabel(
                QString("<b>%1. %2</b>").arg(i + 1).arg(bc.name), m_dynamicArea);
            dynamicLayout->addWidget(groupLabel);

            auto *form = new QFormLayout;
            form->addRow("ID:", new QLabel(bc.id, m_dynamicArea));
            form->addRow("Type:", new QLabel(toString(bc.type), m_dynamicArea));
            form->addRow("Geometry:", new QLabel(bc.target.geometryName, m_dynamicArea));
            form->addRow("Face Group:", new QLabel(bc.target.faceGroupName, m_dynamicArea));
            form->addRow("Material ID:", new QLabel(bc.materialId, m_dynamicArea));
            form->addRow("Enabled:", new QLabel(bc.enabled ? "Yes" : "No", m_dynamicArea));

            dynamicLayout->addLayout(form);
        }
    }
}

void PropertyPanel::showLoadCategory(const std::vector<Load> &loads)
{
    clearAll();
    m_selectionValue->setText("Loads");

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);

    if (loads.empty()) {
        dynamicLayout->addWidget(new QLabel("No loads defined.", m_dynamicArea));
    } else {
        for (size_t i = 0; i < loads.size(); ++i) {
            const Load &ld = loads[i];
            auto *groupLabel = new QLabel(
                QString("<b>%1. %2</b>").arg(i + 1).arg(ld.name), m_dynamicArea);
            dynamicLayout->addWidget(groupLabel);

            auto *form = new QFormLayout;
            form->addRow("ID:", new QLabel(ld.id, m_dynamicArea));
            form->addRow("Type:", new QLabel(toString(ld.type), m_dynamicArea));
            form->addRow("Boundary Condition ID:", new QLabel(ld.boundaryConditionId, m_dynamicArea));
            form->addRow("Field Name:", new QLabel(ld.fieldName, m_dynamicArea));

            QString valueText = QString::number(ld.value.x);
            if (!ld.value.unit.isEmpty()) {
                valueText += " " + ld.value.unit;
            }
            form->addRow("Value:", new QLabel(valueText, m_dynamicArea));
            form->addRow("Enabled:", new QLabel(ld.enabled ? "Yes" : "No", m_dynamicArea));

            dynamicLayout->addLayout(form);
        }
    }
}

void PropertyPanel::showMaterial(const Material &material)
{
    clearAll();
    m_selectionValue->setText(material.name);
    m_typeValue->setText("Material");
    m_nameValue->setText(material.name);

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(material.id, m_dynamicArea));
    form->addRow("Domain:", new QLabel(toString(material.domain), m_dynamicArea));
    form->addRow("Viscosity Model:", new QLabel(toString(material.viscosityModel), m_dynamicArea));
    form->addRow("Density Enabled:", new QLabel(material.hasDensity ? "Yes" : "No", m_dynamicArea));
    form->addRow("Density:", new QLabel(
        QString::number(material.density) + " " + material.densityUnit, m_dynamicArea));
    form->addRow("Dynamic Viscosity Enabled:", new QLabel(
        material.hasDynamicViscosity ? "Yes" : "No", m_dynamicArea));
    form->addRow("Dynamic Viscosity:", new QLabel(
        QString::number(material.dynamicViscosity) + " " + material.dynamicViscosityUnit, m_dynamicArea));
    form->addRow("Kinematic Viscosity Enabled:", new QLabel(
        material.hasKinematicViscosity ? "Yes" : "No", m_dynamicArea));
    form->addRow("Kinematic Viscosity:", new QLabel(
        QString::number(material.kinematicViscosity) + " " + material.kinematicViscosityUnit, m_dynamicArea));
    dynamicLayout->addLayout(form);
}

void PropertyPanel::showBoundaryCondition(const BoundaryCondition &boundaryCondition)
{
    clearAll();
    m_selectionValue->setText(boundaryCondition.name);
    m_typeValue->setText("Boundary Condition");
    m_nameValue->setText(boundaryCondition.name);
    m_sourceGeometryValue->setText(boundaryCondition.target.geometryName);

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(boundaryCondition.id, m_dynamicArea));
    form->addRow("Type:", new QLabel(toString(boundaryCondition.type), m_dynamicArea));
    form->addRow("Target Kind:", new QLabel(toString(boundaryCondition.target.kind), m_dynamicArea));
    form->addRow("Geometry:", new QLabel(boundaryCondition.target.geometryName, m_dynamicArea));
    form->addRow("Face Group ID:", new QLabel(boundaryCondition.target.faceGroupId, m_dynamicArea));
    form->addRow("Face Group:", new QLabel(boundaryCondition.target.faceGroupName, m_dynamicArea));
    form->addRow("Mesh Boundary:", new QLabel(boundaryCondition.target.meshBoundaryName, m_dynamicArea));
    form->addRow("Material ID:", new QLabel(boundaryCondition.materialId, m_dynamicArea));
    form->addRow("Enabled:", new QLabel(boundaryCondition.enabled ? "Yes" : "No", m_dynamicArea));
    dynamicLayout->addLayout(form);
}

void PropertyPanel::showLoad(const Load &load)
{
    clearAll();
    m_selectionValue->setText(load.name);
    m_typeValue->setText("Load");
    m_nameValue->setText(load.name);

    QString valueText;
    if (load.value.kind == LoadValueKind::Vector3) {
        valueText = QString("(%1, %2, %3)")
            .arg(load.value.x)
            .arg(load.value.y)
            .arg(load.value.z);
    } else {
        valueText = QString::number(load.value.x);
    }
    if (!load.value.unit.isEmpty()) {
        valueText += " " + load.value.unit;
    }

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(load.id, m_dynamicArea));
    form->addRow("Type:", new QLabel(toString(load.type), m_dynamicArea));
    form->addRow("Boundary Condition ID:", new QLabel(load.boundaryConditionId, m_dynamicArea));
    form->addRow("Field Name:", new QLabel(load.fieldName, m_dynamicArea));
    form->addRow("Value Kind:", new QLabel(toString(load.value.kind), m_dynamicArea));
    form->addRow("Value:", new QLabel(valueText, m_dynamicArea));
    form->addRow("Enabled:", new QLabel(load.enabled ? "Yes" : "No", m_dynamicArea));
    dynamicLayout->addLayout(form);
}

void PropertyPanel::showSolverCategory(const SimulationCase &simulationCase)
{
    clearAll();
    m_selectionValue->setText(simulationCase.name);

    resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(m_dynamicArea);

    auto *form = new QFormLayout;
    form->addRow("Case ID:", new QLabel(simulationCase.id, m_dynamicArea));
    form->addRow("Source Geometry:", new QLabel(simulationCase.sourceGeometryName, m_dynamicArea));
    form->addRow("Mesh:", new QLabel(simulationCase.meshName, m_dynamicArea));
    form->addRow("Solver Type:", new QLabel(toString(simulationCase.solverType), m_dynamicArea));
    form->addRow("Turbulence Model:", new QLabel(toString(simulationCase.turbulenceModel), m_dynamicArea));
    form->addRow("End Time:", new QLabel(QString::number(simulationCase.runControl.endTime), m_dynamicArea));
    form->addRow("Time Step:", new QLabel(QString::number(simulationCase.runControl.timeStep), m_dynamicArea));
    form->addRow("Write Interval:", new QLabel(QString::number(simulationCase.runControl.writeInterval), m_dynamicArea));
    form->addRow("Post Processing:", new QLabel(simulationCase.postProcessingTool, m_dynamicArea));
    form->addRow("Materials:", new QLabel(QString::number(simulationCase.materials.size()), m_dynamicArea));
    form->addRow("Boundary Conditions:", new QLabel(QString::number(simulationCase.boundaryConditions.size()), m_dynamicArea));
    form->addRow("Loads:", new QLabel(QString::number(simulationCase.loads.size()), m_dynamicArea));
    dynamicLayout->addLayout(form);

    dynamicLayout->addWidget(new QLabel(
        "<i>Simulation export uses the current project model.</i>", m_dynamicArea));
}
