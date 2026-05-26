#include "PropertyPanel.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "result/ResultObject.h"
#include "ui/property/FaceGroupPropertyView.h"
#include "ui/property/GeometryPropertyView.h"
#include "ui/property/PickPropertyView.h"
#include "ui/property/ResultPropertyView.h"
#include "ui/property/SolverPropertyView.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace
{
QLabel *createValueLabel(const QString &text, QWidget *parent, const QString &objectName)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QFormLayout *createSection(QVBoxLayout *layout, const QString &title, QWidget *parent, const QString &objectName)
{
    auto *group = new QGroupBox(title, parent);
    group->setObjectName(objectName);

    auto *form = new QFormLayout(group);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(5);

    layout->addWidget(group);
    return form;
}

QWidget *createDetailsSection(QVBoxLayout *layout, QWidget *parent)
{
    auto *group = new QGroupBox("Solver / Result Details", parent);
    group->setObjectName("property.section.details");
    layout->addWidget(group);
    return group;
}
}

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    m_selectionValue = createValueLabel("None", this, "property.selection.value");
    m_typeValue = createValueLabel("-", this, "property.type.value");
    m_nameValue = createValueLabel("-", this, "property.name.value");
    m_radiusValue = createValueLabel("-", this, "property.radius.value");
    m_lengthValue = createValueLabel("-", this, "property.length.value");
    m_widthValue = createValueLabel("-", this, "property.width.value");
    m_heightValue = createValueLabel("-", this, "property.height.value");
    m_sourceGeometryValue = createValueLabel("-", this, "property.sourceGeometry.value");
    m_sourceGeometryTypeValue = createValueLabel("-", this, "property.sourceType.value");
    m_sourceStepFileValue = createValueLabel("-", this, "property.sourceStep.value");
    m_meshFileValue = createValueLabel("-", this, "property.meshFile.value");
    m_nodeCountValue = createValueLabel("-", this, "property.nodeCount.value");
    m_tetraCountValue = createValueLabel("-", this, "property.tetraCount.value");
    m_createdAtValue = createValueLabel("-", this, "property.createdAt.value");

    QFormLayout *identityForm = createSection(m_mainLayout, "Selection", this, "property.section.selection");
    identityForm->addRow("Selection", m_selectionValue);
    identityForm->addRow("Type", m_typeValue);
    identityForm->addRow("Name", m_nameValue);
    identityForm->addRow("Created At", m_createdAtValue);
    identityForm->addRow("Source Geometry", m_sourceGeometryValue);
    identityForm->addRow("Source Type", m_sourceGeometryTypeValue);

    QFormLayout *geometryForm = createSection(m_mainLayout, "Geometry", this, "property.section.geometry");
    geometryForm->addRow("Radius", m_radiusValue);
    geometryForm->addRow("Length", m_lengthValue);
    geometryForm->addRow("Width", m_widthValue);
    geometryForm->addRow("Height", m_heightValue);

    QFormLayout *sourceForm = createSection(m_mainLayout, "Mesh", this, "property.section.sourceMesh");
    sourceForm->addRow("Source STEP", m_sourceStepFileValue);
    sourceForm->addRow("Mesh File", m_meshFileValue);
    sourceForm->addRow("Node Count", m_nodeCountValue);
    sourceForm->addRow("Tetra Count", m_tetraCountValue);

    m_dynamicArea = createDetailsSection(m_mainLayout, this);
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

QWidget *PropertyPanel::resetDynamicArea(bool visible)
{
    if (m_dynamicArea) {
        m_mainLayout->removeWidget(m_dynamicArea);
        delete m_dynamicArea;
        m_dynamicArea = nullptr;
    }

    m_dynamicArea = new QGroupBox("Solver / Result Details", this);
    m_dynamicArea->setObjectName("property.section.details");
    m_dynamicArea->setVisible(visible);
    m_mainLayout->insertWidget(m_mainLayout->count() - 1, m_dynamicArea);
    return m_dynamicArea;
}

void PropertyPanel::showEmptySelection()
{
    clearAll();
    resetDynamicArea(false);
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

    resetDynamicArea(false);
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

    resetDynamicArea(false);
}

void PropertyPanel::showGeometryObject(const GeometryObject &geometry)
{
    clearAll();
    m_selectionValue->setText(geometry.name);
    m_typeValue->setText(geometry.type);
    m_nameValue->setText(geometry.name);
    m_sourceStepFileValue->setText(geometry.stepFile);

    resetDynamicArea();
    GeometryPropertyView::populate(m_dynamicArea, geometry);
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

    resetDynamicArea(false);
}

void PropertyPanel::showFaceGroup(const FaceGroup &faceGroup)
{
    clearAll();
    m_selectionValue->setText(faceGroup.id);
    m_typeValue->setText("Face Group");
    m_nameValue->setText(faceGroup.name);
    m_sourceGeometryValue->setText(faceGroup.geometryName);

    resetDynamicArea();
    FaceGroupPropertyView::populate(m_dynamicArea, faceGroup);
}

void PropertyPanel::showPickState(PickMode mode, const QString &geometryName, const std::vector<int> &faceIndices)
{
    clearAll();
    m_selectionValue->setText("Pick");
    m_typeValue->setText("Picking");
    m_nameValue->setText(pickModeName(mode));
    m_sourceGeometryValue->setText(geometryName);

    resetDynamicArea();
    PickPropertyView::populate(m_dynamicArea, mode, geometryName, faceIndices);
}

void PropertyPanel::showMaterialCategory(const std::vector<Material> &materials)
{
    clearAll();
    m_selectionValue->setText("Materials");

    resetDynamicArea();
    SolverPropertyView::populateMaterialCategory(m_dynamicArea, materials);
}

void PropertyPanel::showBoundaryConditionCategory(const std::vector<BoundaryCondition> &boundaryConditions)
{
    clearAll();
    m_selectionValue->setText("Boundary Conditions");

    resetDynamicArea();
    SolverPropertyView::populateBoundaryConditionCategory(m_dynamicArea, boundaryConditions);
}

void PropertyPanel::showLoadCategory(const std::vector<Load> &loads)
{
    clearAll();
    m_selectionValue->setText("Loads");

    resetDynamicArea();
    SolverPropertyView::populateLoadCategory(m_dynamicArea, loads);
}

void PropertyPanel::showMaterial(const Material &material)
{
    clearAll();
    m_selectionValue->setText(material.name);
    m_typeValue->setText("Material");
    m_nameValue->setText(material.name);

    resetDynamicArea();
    SolverPropertyView::populateMaterial(m_dynamicArea, material);
}

void PropertyPanel::showBoundaryCondition(const BoundaryCondition &boundaryCondition)
{
    clearAll();
    m_selectionValue->setText(boundaryCondition.name);
    m_typeValue->setText("Boundary Condition");
    m_nameValue->setText(boundaryCondition.name);
    m_sourceGeometryValue->setText(boundaryCondition.target.geometryName);

    resetDynamicArea();
    SolverPropertyView::populateBoundaryCondition(m_dynamicArea, boundaryCondition);
}

void PropertyPanel::showLoad(const Load &load)
{
    clearAll();
    m_selectionValue->setText(load.name);
    m_typeValue->setText("Load");
    m_nameValue->setText(load.name);

    resetDynamicArea();
    SolverPropertyView::populateLoad(m_dynamicArea, load);
}

void PropertyPanel::showSolverCategory(const SimulationCase &simulationCase)
{
    clearAll();
    m_selectionValue->setText(simulationCase.name);

    resetDynamicArea();
    SolverPropertyView::populateSolverCategory(m_dynamicArea, simulationCase);
}

void PropertyPanel::showResultCategory(const std::vector<ResultObject> &results)
{
    clearAll();
    m_selectionValue->setText("Results");
    m_typeValue->setText("Result Category");

    QWidget *dynamicArea = resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(dynamicArea);
    if (results.empty()) {
        dynamicLayout->addWidget(new QLabel("No solver results.", dynamicArea));
        return;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        const ResultObject &resultObject = results[i];
        dynamicLayout->addWidget(
            new QLabel(QString("<b>%1. %2</b> - %3")
                .arg(i + 1)
                .arg(resultObject.name)
                .arg(resultObject.success ? "Success" : "Failed"),
                dynamicArea)
        );
    }
}

void PropertyPanel::showResult(const ResultObject &resultObject)
{
    clearAll();
    m_selectionValue->setText(resultObject.name);
    m_typeValue->setText("Result");
    m_nameValue->setText(resultObject.name);
    m_sourceStepFileValue->setText(resultObject.casePath);
    m_createdAtValue->setText(resultObject.createdAt);

    resetDynamicArea();
    ResultPropertyView::populate(m_dynamicArea, resultObject);
}
