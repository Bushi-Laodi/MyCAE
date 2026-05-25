#include "ui/property/SolverPropertyView.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace
{
QString loadValueText(const Load &load)
{
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
    return valueText;
}
}

void SolverPropertyView::populateMaterialCategory(QWidget *parent, const std::vector<Material> &materials)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    if (materials.empty()) {
        dynamicLayout->addWidget(new QLabel("No materials defined.", parent));
        return;
    }

    for (size_t i = 0; i < materials.size(); ++i) {
        const Material &mat = materials[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(mat.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(mat.id, parent));
        form->addRow("Domain:", new QLabel(toString(mat.domain), parent));
        form->addRow("Viscosity Model:", new QLabel(toString(mat.viscosityModel), parent));
        if (mat.hasDensity) {
            form->addRow("Density:", new QLabel(QString::number(mat.density) + " " + mat.densityUnit, parent));
        }
        if (mat.hasDynamicViscosity) {
            form->addRow(
                "Dynamic Viscosity:",
                new QLabel(QString::number(mat.dynamicViscosity) + " " + mat.dynamicViscosityUnit, parent)
            );
        }
        if (mat.hasKinematicViscosity) {
            form->addRow(
                "Kinematic Viscosity:",
                new QLabel(QString::number(mat.kinematicViscosity) + " " + mat.kinematicViscosityUnit, parent)
            );
        }
        dynamicLayout->addLayout(form);
    }
}

void SolverPropertyView::populateBoundaryConditionCategory(
    QWidget *parent,
    const std::vector<BoundaryCondition> &boundaryConditions
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    if (boundaryConditions.empty()) {
        dynamicLayout->addWidget(new QLabel("No boundary conditions defined.", parent));
        return;
    }

    for (size_t i = 0; i < boundaryConditions.size(); ++i) {
        const BoundaryCondition &bc = boundaryConditions[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(bc.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(bc.id, parent));
        form->addRow("Type:", new QLabel(toString(bc.type), parent));
        form->addRow("Geometry:", new QLabel(bc.target.geometryName, parent));
        form->addRow("Face Group:", new QLabel(bc.target.faceGroupName, parent));
        form->addRow("Material ID:", new QLabel(bc.materialId, parent));
        form->addRow("Enabled:", new QLabel(bc.enabled ? "Yes" : "No", parent));
        dynamicLayout->addLayout(form);
    }
}

void SolverPropertyView::populateLoadCategory(QWidget *parent, const std::vector<Load> &loads)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    if (loads.empty()) {
        dynamicLayout->addWidget(new QLabel("No loads defined.", parent));
        return;
    }

    for (size_t i = 0; i < loads.size(); ++i) {
        const Load &ld = loads[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(ld.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(ld.id, parent));
        form->addRow("Type:", new QLabel(toString(ld.type), parent));
        form->addRow("Boundary Condition ID:", new QLabel(ld.boundaryConditionId, parent));
        form->addRow("Field Name:", new QLabel(ld.fieldName, parent));
        form->addRow("Value:", new QLabel(loadValueText(ld), parent));
        form->addRow("Enabled:", new QLabel(ld.enabled ? "Yes" : "No", parent));
        dynamicLayout->addLayout(form);
    }
}

void SolverPropertyView::populateMaterial(QWidget *parent, const Material &material)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(material.id, parent));
    form->addRow("Domain:", new QLabel(toString(material.domain), parent));
    form->addRow("Viscosity Model:", new QLabel(toString(material.viscosityModel), parent));
    form->addRow("Density Enabled:", new QLabel(material.hasDensity ? "Yes" : "No", parent));
    form->addRow("Density:", new QLabel(QString::number(material.density) + " " + material.densityUnit, parent));
    form->addRow("Dynamic Viscosity Enabled:", new QLabel(material.hasDynamicViscosity ? "Yes" : "No", parent));
    form->addRow(
        "Dynamic Viscosity:",
        new QLabel(QString::number(material.dynamicViscosity) + " " + material.dynamicViscosityUnit, parent)
    );
    form->addRow("Kinematic Viscosity Enabled:", new QLabel(material.hasKinematicViscosity ? "Yes" : "No", parent));
    form->addRow(
        "Kinematic Viscosity:",
        new QLabel(QString::number(material.kinematicViscosity) + " " + material.kinematicViscosityUnit, parent)
    );
    dynamicLayout->addLayout(form);
}

void SolverPropertyView::populateBoundaryCondition(QWidget *parent, const BoundaryCondition &boundaryCondition)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(boundaryCondition.id, parent));
    form->addRow("Type:", new QLabel(toString(boundaryCondition.type), parent));
    form->addRow("Target Kind:", new QLabel(toString(boundaryCondition.target.kind), parent));
    form->addRow("Geometry:", new QLabel(boundaryCondition.target.geometryName, parent));
    form->addRow("Face Group ID:", new QLabel(boundaryCondition.target.faceGroupId, parent));
    form->addRow("Face Group:", new QLabel(boundaryCondition.target.faceGroupName, parent));
    form->addRow("Mesh Boundary:", new QLabel(boundaryCondition.target.meshBoundaryName, parent));
    form->addRow("Material ID:", new QLabel(boundaryCondition.materialId, parent));
    form->addRow("Enabled:", new QLabel(boundaryCondition.enabled ? "Yes" : "No", parent));
    dynamicLayout->addLayout(form);
}

void SolverPropertyView::populateLoad(QWidget *parent, const Load &load)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(load.id, parent));
    form->addRow("Type:", new QLabel(toString(load.type), parent));
    form->addRow("Boundary Condition ID:", new QLabel(load.boundaryConditionId, parent));
    form->addRow("Field Name:", new QLabel(load.fieldName, parent));
    form->addRow("Value Kind:", new QLabel(toString(load.value.kind), parent));
    form->addRow("Value:", new QLabel(loadValueText(load), parent));
    form->addRow("Enabled:", new QLabel(load.enabled ? "Yes" : "No", parent));
    dynamicLayout->addLayout(form);
}

void SolverPropertyView::populateSolverCategory(QWidget *parent, const SimulationCase &simulationCase)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    auto *form = new QFormLayout;
    form->addRow("Case ID:", new QLabel(simulationCase.id, parent));
    form->addRow("Source Geometry:", new QLabel(simulationCase.sourceGeometryName, parent));
    form->addRow("Mesh:", new QLabel(simulationCase.meshName, parent));
    form->addRow("Solver Type:", new QLabel(toString(simulationCase.solverType), parent));
    form->addRow("Turbulence Model:", new QLabel(toString(simulationCase.turbulenceModel), parent));
    form->addRow("End Time:", new QLabel(QString::number(simulationCase.runControl.endTime), parent));
    form->addRow("Time Step:", new QLabel(QString::number(simulationCase.runControl.timeStep), parent));
    form->addRow("Write Interval:", new QLabel(QString::number(simulationCase.runControl.writeInterval), parent));
    form->addRow("Post Processing:", new QLabel(simulationCase.postProcessingTool, parent));
    form->addRow("Materials:", new QLabel(QString::number(simulationCase.materials.size()), parent));
    form->addRow(
        "Boundary Conditions:",
        new QLabel(QString::number(simulationCase.boundaryConditions.size()), parent)
    );
    form->addRow("Loads:", new QLabel(QString::number(simulationCase.loads.size()), parent));
    dynamicLayout->addLayout(form);

    dynamicLayout->addWidget(new QLabel("<i>Simulation export uses the current project model.</i>", parent));
}
