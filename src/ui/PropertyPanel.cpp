#include "PropertyPanel.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "geometry/SphereGeometry.h"
#include "mesh/MeshObject.h"
#include "result/ResultObject.h"
#include "solver/BoundaryBindingInspector.h"
#include "ui/property/FaceGroupPropertyView.h"
#include "ui/property/GeometryPropertyView.h"
#include "ui/property/MeshPropertyView.h"
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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

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
    auto *group = new QGroupBox(zh(u8"求解 / 结果详情"), parent);
    group->setObjectName("property.section.details");
    layout->addWidget(group);
    return group;
}

QString centerText(double x, double y, double z, const QString &unit)
{
    return QString("(%1, %2, %3) %4")
        .arg(x, 0, 'g', 6)
        .arg(y, 0, 'g', 6)
        .arg(z, 0, 'g', 6)
        .arg(unit);
}
}

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    m_selectionValue = createValueLabel(zh(u8"无"), this, "property.selection.value");
    m_typeValue = createValueLabel("-", this, "property.type.value");
    m_nameValue = createValueLabel("-", this, "property.name.value");
    m_radiusValue = createValueLabel("-", this, "property.radius.value");
    m_lengthValue = createValueLabel("-", this, "property.length.value");
    m_widthValue = createValueLabel("-", this, "property.width.value");
    m_heightValue = createValueLabel("-", this, "property.height.value");
    m_centerValue = createValueLabel("-", this, "property.center.value");
    m_sourceGeometryValue = createValueLabel("-", this, "property.sourceGeometry.value");
    m_sourceGeometryTypeValue = createValueLabel("-", this, "property.sourceType.value");
    m_sourceStepFileValue = createValueLabel("-", this, "property.sourceStep.value");
    m_meshFileValue = createValueLabel("-", this, "property.meshFile.value");
    m_nodeCountValue = createValueLabel("-", this, "property.nodeCount.value");
    m_tetraCountValue = createValueLabel("-", this, "property.tetraCount.value");
    m_createdAtValue = createValueLabel("-", this, "property.createdAt.value");

    QFormLayout *identityForm = createSection(m_mainLayout, zh(u8"选择"), this, "property.section.selection");
    identityForm->addRow(zh(u8"选择"), m_selectionValue);
    identityForm->addRow(zh(u8"类型"), m_typeValue);
    identityForm->addRow(zh(u8"名称"), m_nameValue);
    identityForm->addRow(zh(u8"创建时间"), m_createdAtValue);
    identityForm->addRow(zh(u8"源几何"), m_sourceGeometryValue);
    identityForm->addRow(zh(u8"源类型"), m_sourceGeometryTypeValue);

    QFormLayout *geometryForm = createSection(m_mainLayout, zh(u8"几何"), this, "property.section.geometry");
    geometryForm->addRow(zh(u8"中心"), m_centerValue);
    geometryForm->addRow(zh(u8"半径"), m_radiusValue);
    geometryForm->addRow(zh(u8"长度"), m_lengthValue);
    geometryForm->addRow(zh(u8"宽度"), m_widthValue);
    geometryForm->addRow(zh(u8"高度"), m_heightValue);

    QFormLayout *sourceForm = createSection(m_mainLayout, zh(u8"网格"), this, "property.section.sourceMesh");
    sourceForm->addRow(zh(u8"源 STEP"), m_sourceStepFileValue);
    sourceForm->addRow(zh(u8"网格文件"), m_meshFileValue);
    sourceForm->addRow(zh(u8"节点数"), m_nodeCountValue);
    sourceForm->addRow(zh(u8"四面体数"), m_tetraCountValue);

    m_dynamicArea = createDetailsSection(m_mainLayout, this);
    m_mainLayout->addStretch();
}

void PropertyPanel::clearAll()
{
    m_selectionValue->setText(zh(u8"无"));
    m_typeValue->setText("-");
    m_nameValue->setText("-");
    m_radiusValue->setText("-");
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_centerValue->setText("-");
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

    m_dynamicArea = new QGroupBox(zh(u8"求解 / 结果详情"), this);
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
    showBoxGeometry(box, GeometryObject{}, GeometryPropertyDetails{});
}

void PropertyPanel::showBoxGeometry(
    const BoxGeometry &box,
    const GeometryObject &geometry,
    const GeometryPropertyDetails &details
)
{
    const QString suffix = " " + box.unit;
    m_selectionValue->setText(box.name);
    m_typeValue->setText(zh(u8"长方体"));
    m_nameValue->setText(box.name);
    m_centerValue->setText(centerText(box.centerX, box.centerY, box.centerZ, box.unit));
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

    if (geometry.name.isEmpty()) {
        resetDynamicArea(false);
    } else {
        resetDynamicArea();
        GeometryPropertyView::populate(m_dynamicArea, geometry, details);
    }
}

void PropertyPanel::showCylinderGeometry(const CylinderGeometry &cylinder)
{
    showCylinderGeometry(cylinder, GeometryObject{}, GeometryPropertyDetails{});
}

void PropertyPanel::showCylinderGeometry(
    const CylinderGeometry &cylinder,
    const GeometryObject &geometry,
    const GeometryPropertyDetails &details
)
{
    const QString suffix = " " + cylinder.unit;
    m_selectionValue->setText(cylinder.name);
    m_typeValue->setText(zh(u8"圆柱体"));
    m_nameValue->setText(cylinder.name);
    m_centerValue->setText(centerText(cylinder.centerX, cylinder.centerY, cylinder.centerZ, cylinder.unit));
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

    if (geometry.name.isEmpty()) {
        resetDynamicArea(false);
    } else {
        resetDynamicArea();
        GeometryPropertyView::populate(m_dynamicArea, geometry, details);
    }
}

void PropertyPanel::showSphereGeometry(const SphereGeometry &sphere)
{
    showSphereGeometry(sphere, GeometryObject{}, GeometryPropertyDetails{});
}

void PropertyPanel::showSphereGeometry(
    const SphereGeometry &sphere,
    const GeometryObject &geometry,
    const GeometryPropertyDetails &details
)
{
    const QString suffix = " " + sphere.unit;
    m_selectionValue->setText(sphere.name);
    m_typeValue->setText(zh(u8"球体"));
    m_nameValue->setText(sphere.name);
    m_centerValue->setText(centerText(sphere.centerX, sphere.centerY, sphere.centerZ, sphere.unit));
    m_radiusValue->setText(QString::number(sphere.radius) + suffix);
    m_lengthValue->setText("-");
    m_widthValue->setText("-");
    m_heightValue->setText("-");
    m_sourceGeometryValue->setText("-");
    m_sourceGeometryTypeValue->setText("-");
    m_sourceStepFileValue->setText(sphere.occStepFile);
    m_meshFileValue->setText("-");
    m_nodeCountValue->setText("-");
    m_tetraCountValue->setText("-");
    m_createdAtValue->setText("-");

    if (geometry.name.isEmpty()) {
        resetDynamicArea(false);
    } else {
        resetDynamicArea();
        GeometryPropertyView::populate(m_dynamicArea, geometry, details);
    }
}

void PropertyPanel::showGeometryObject(const GeometryObject &geometry)
{
    showGeometryObject(geometry, GeometryPropertyDetails{});
}

void PropertyPanel::showGeometryObject(const GeometryObject &geometry, const GeometryPropertyDetails &details)
{
    clearAll();
    m_selectionValue->setText(geometry.name);
    m_typeValue->setText(geometry.type);
    m_nameValue->setText(geometry.name);
    m_sourceStepFileValue->setText(geometry.stepFile);
    m_centerValue->setText(details.center.isEmpty() ? "-" : details.center);

    resetDynamicArea();
    GeometryPropertyView::populate(m_dynamicArea, geometry, details);
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
    m_centerValue->setText("-");
    m_sourceGeometryValue->setText(meshObject.sourceGeometryName);
    m_sourceGeometryTypeValue->setText(meshObject.sourceGeometryType);
    m_sourceStepFileValue->setText(meshObject.sourceStepFile);
    m_meshFileValue->setText(meshObject.mshFile);
    m_nodeCountValue->setText(QString::number(meshObject.nodeCount));
    m_tetraCountValue->setText(QString::number(meshObject.tetraCount));
    m_createdAtValue->setText(meshObject.createdAt);

    resetDynamicArea();
    MeshPropertyView::populate(m_dynamicArea, meshObject);
}

void PropertyPanel::showFaceGroup(const FaceGroup &faceGroup)
{
    const std::vector<BoundaryCondition> emptyBoundaryConditions;
    const std::vector<Load> emptyLoads;
    showFaceGroup(faceGroup, emptyBoundaryConditions, emptyLoads);
}

void PropertyPanel::showFaceGroup(
    const FaceGroup &faceGroup,
    const std::vector<BoundaryCondition> &boundaryConditions,
    const std::vector<Load> &loads
)
{
    clearAll();
    m_selectionValue->setText(faceGroup.id);
    m_typeValue->setText(zh(u8"面组"));
    m_nameValue->setText(faceGroup.name);
    m_sourceGeometryValue->setText(faceGroup.geometryName);

    resetDynamicArea();
    FaceGroupPropertyView::populate(
        m_dynamicArea,
        faceGroup,
        BoundaryBindingInspector::summarizeFaceGroup(faceGroup, boundaryConditions, loads)
    );
}

void PropertyPanel::showPickState(PickMode mode, const QString &geometryName, const std::vector<int> &faceIndices)
{
    clearAll();
    m_selectionValue->setText(zh(u8"拾取"));
    m_typeValue->setText(zh(u8"拾取"));
    m_nameValue->setText(pickModeName(mode));
    m_sourceGeometryValue->setText(geometryName);

    resetDynamicArea();
    PickPropertyView::populate(m_dynamicArea, mode, geometryName, faceIndices);
}

void PropertyPanel::showMaterialCategory(const std::vector<Material> &materials)
{
    clearAll();
    m_selectionValue->setText(zh(u8"材料"));

    resetDynamicArea();
    SolverPropertyView::populateMaterialCategory(m_dynamicArea, materials);
}

void PropertyPanel::showSectionAssignmentCategory(const std::vector<SectionAssignment> &sectionAssignments)
{
    clearAll();
    m_selectionValue->setText(zh(u8"材料分区"));
    m_typeValue->setText(zh(u8"结构工况数据"));

    resetDynamicArea();
    SolverPropertyView::populateSectionAssignmentCategory(m_dynamicArea, sectionAssignments);
}

void PropertyPanel::showBoundaryConditionCategory(const std::vector<BoundaryCondition> &boundaryConditions)
{
    clearAll();
    m_selectionValue->setText(zh(u8"边界条件"));

    resetDynamicArea();
    SolverPropertyView::populateBoundaryConditionCategory(m_dynamicArea, boundaryConditions);
}

void PropertyPanel::showLoadCategory(const std::vector<Load> &loads)
{
    clearAll();
    m_selectionValue->setText(zh(u8"载荷"));

    resetDynamicArea();
    SolverPropertyView::populateLoadCategory(m_dynamicArea, loads);
}

void PropertyPanel::showMaterial(const Material &material)
{
    clearAll();
    m_selectionValue->setText(material.name);
    m_typeValue->setText(zh(u8"材料"));
    m_nameValue->setText(material.name);

    resetDynamicArea();
    SolverPropertyView::populateMaterial(m_dynamicArea, material);
}

void PropertyPanel::showSectionAssignment(const SectionAssignment &sectionAssignment)
{
    clearAll();
    m_selectionValue->setText(sectionAssignment.name);
    m_typeValue->setText(zh(u8"材料分区"));
    m_nameValue->setText(sectionAssignment.name);
    m_sourceGeometryValue->setText(sectionAssignment.geometryName);
    m_meshFileValue->setText(sectionAssignment.meshName);

    resetDynamicArea();
    SolverPropertyView::populateSectionAssignment(m_dynamicArea, sectionAssignment);
}

void PropertyPanel::showBoundaryCondition(const BoundaryCondition &boundaryCondition)
{
    const std::vector<FaceGroup> emptyFaceGroups;
    const std::vector<Load> emptyLoads;
    showBoundaryCondition(boundaryCondition, emptyFaceGroups, emptyLoads);
}

void PropertyPanel::showBoundaryCondition(
    const BoundaryCondition &boundaryCondition,
    const std::vector<FaceGroup> &faceGroups,
    const std::vector<Load> &loads
)
{
    clearAll();
    m_selectionValue->setText(boundaryCondition.name);
    m_typeValue->setText(zh(u8"边界条件"));
    m_nameValue->setText(boundaryCondition.name);
    m_sourceGeometryValue->setText(boundaryCondition.target.geometryName);

    resetDynamicArea();
    SolverPropertyView::populateBoundaryCondition(
        m_dynamicArea,
        boundaryCondition,
        BoundaryBindingInspector::summarizeBoundaryCondition(boundaryCondition, faceGroups, loads)
    );
}

void PropertyPanel::showLoad(const Load &load)
{
    clearAll();
    m_selectionValue->setText(load.name);
    m_typeValue->setText(zh(u8"载荷"));
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
    m_selectionValue->setText(zh(u8"结果"));
    m_typeValue->setText(zh(u8"结果分类"));

    QWidget *dynamicArea = resetDynamicArea();
    auto *dynamicLayout = new QVBoxLayout(dynamicArea);
    if (results.empty()) {
        dynamicLayout->addWidget(new QLabel(zh(u8"暂无求解结果。"), dynamicArea));
        return;
    }

    for (size_t i = 0; i < results.size(); ++i) {
        const ResultObject &resultObject = results[i];
        dynamicLayout->addWidget(
            new QLabel(QString("<b>%1. %2</b> - %3")
                .arg(i + 1)
                .arg(resultObject.name)
                .arg(resultObject.success ? zh(u8"成功") : zh(u8"失败")),
                dynamicArea)
        );
    }
}

void PropertyPanel::showResult(const ResultObject &resultObject)
{
    clearAll();
    m_selectionValue->setText(resultObject.name);
    m_typeValue->setText(zh(u8"结果"));
    m_nameValue->setText(resultObject.name);
    m_sourceStepFileValue->setText(resultObject.casePath);
    m_createdAtValue->setText(resultObject.createdAt);

    resetDynamicArea();
    ResultPropertyView::populate(m_dynamicArea, resultObject);
}
