#include "ui/property/SolverPropertyView.h"

#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString yesNoText(bool value)
{
    return value ? zh(u8"是") : zh(u8"否");
}

QString materialDomainText(MaterialDomain domain)
{
    return domain == MaterialDomain::Solid ? zh(u8"固体") : zh(u8"流体");
}

QString viscosityModelText(ViscosityModel model)
{
    Q_UNUSED(model);
    return zh(u8"牛顿流体");
}

QString boundaryTargetKindText(BoundaryTargetKind kind)
{
    return kind == BoundaryTargetKind::MeshBoundary ? zh(u8"网格边界") : zh(u8"几何面组");
}

QString boundaryConditionTypeText(BoundaryConditionType type)
{
    switch (type) {
    case BoundaryConditionType::Wall:
        return zh(u8"壁面");
    case BoundaryConditionType::VelocityInlet:
        return zh(u8"速度入口");
    case BoundaryConditionType::PressureInlet:
        return zh(u8"压力入口");
    case BoundaryConditionType::PressureOutlet:
        return zh(u8"压力出口");
    case BoundaryConditionType::Symmetry:
        return zh(u8"对称");
    case BoundaryConditionType::Unknown:
        return zh(u8"未知");
    }
    return zh(u8"未知");
}

QString loadTypeText(LoadType type)
{
    switch (type) {
    case LoadType::Velocity:
        return zh(u8"速度");
    case LoadType::Pressure:
        return zh(u8"压力");
    case LoadType::BodyForce:
        return zh(u8"体力");
    case LoadType::Unknown:
        return zh(u8"未知");
    }
    return zh(u8"未知");
}

QString loadValueKindText(LoadValueKind kind)
{
    return kind == LoadValueKind::Vector3 ? zh(u8"三维向量") : zh(u8"标量");
}

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

QLabel *warningLabel(QWidget *parent, const QString &text)
{
    auto *label = valueLabel(text, parent);
    label->setStyleSheet(
        "QLabel {"
        "  color: #92400e;"
        "  background: #fffbeb;"
        "  border: 1px solid #fcd34d;"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "}"
    );
    return label;
}

void appendMaterialExtraProperties(QFormLayout *form, QWidget *parent, const Material &material)
{
    for (const MaterialProperty &property : material.extraProperties) {
        QString value = QString::number(property.value);
        if (!property.unit.trimmed().isEmpty()) {
            value += " " + property.unit;
        }
        form->addRow(property.name + ":", new QLabel(value, parent));
    }
}
}

void SolverPropertyView::populateMaterialCategory(QWidget *parent, const std::vector<Material> &materials)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    if (materials.empty()) {
        dynamicLayout->addWidget(new QLabel(zh(u8"尚未定义材料。"), parent));
        return;
    }

    for (size_t i = 0; i < materials.size(); ++i) {
        const Material &mat = materials[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(mat.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(mat.id, parent));
        form->addRow(zh(u8"物理域:"), new QLabel(materialDomainText(mat.domain), parent));
        form->addRow(zh(u8"黏度模型:"), new QLabel(viscosityModelText(mat.viscosityModel), parent));
        if (mat.hasDensity) {
            form->addRow(zh(u8"密度:"), new QLabel(QString::number(mat.density) + " " + mat.densityUnit, parent));
        }
        if (mat.hasDynamicViscosity) {
            form->addRow(
                zh(u8"动力黏度:"),
                new QLabel(QString::number(mat.dynamicViscosity) + " " + mat.dynamicViscosityUnit, parent)
            );
        }
        if (mat.hasKinematicViscosity) {
            form->addRow(
                zh(u8"运动黏度:"),
                new QLabel(QString::number(mat.kinematicViscosity) + " " + mat.kinematicViscosityUnit, parent)
            );
        }
        appendMaterialExtraProperties(form, parent, mat);
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
        dynamicLayout->addWidget(new QLabel(zh(u8"尚未定义边界条件。"), parent));
        return;
    }

    for (size_t i = 0; i < boundaryConditions.size(); ++i) {
        const BoundaryCondition &bc = boundaryConditions[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(bc.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(bc.id, parent));
        form->addRow(zh(u8"类型:"), new QLabel(boundaryConditionTypeText(bc.type), parent));
        form->addRow(zh(u8"几何:"), new QLabel(bc.target.geometryName, parent));
        form->addRow(zh(u8"面组:"), new QLabel(bc.target.faceGroupName, parent));
        form->addRow(zh(u8"材料 ID:"), new QLabel(bc.materialId, parent));
        form->addRow(zh(u8"启用:"), new QLabel(yesNoText(bc.enabled), parent));
        dynamicLayout->addLayout(form);
    }
}

void SolverPropertyView::populateLoadCategory(QWidget *parent, const std::vector<Load> &loads)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    if (loads.empty()) {
        dynamicLayout->addWidget(new QLabel(zh(u8"尚未定义载荷。"), parent));
        return;
    }

    for (size_t i = 0; i < loads.size(); ++i) {
        const Load &ld = loads[i];
        dynamicLayout->addWidget(new QLabel(QString("<b>%1. %2</b>").arg(i + 1).arg(ld.name), parent));

        auto *form = new QFormLayout;
        form->addRow("ID:", new QLabel(ld.id, parent));
        form->addRow(zh(u8"类型:"), new QLabel(loadTypeText(ld.type), parent));
        form->addRow(zh(u8"边界条件 ID:"), new QLabel(ld.boundaryConditionId, parent));
        form->addRow(zh(u8"场名称:"), new QLabel(ld.fieldName, parent));
        form->addRow(zh(u8"数值:"), new QLabel(loadValueText(ld), parent));
        form->addRow(zh(u8"启用:"), new QLabel(yesNoText(ld.enabled), parent));
        dynamicLayout->addLayout(form);
    }
}

void SolverPropertyView::populateMaterial(QWidget *parent, const Material &material)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(material.id, parent));
    form->addRow(zh(u8"物理域:"), new QLabel(materialDomainText(material.domain), parent));
    form->addRow(zh(u8"黏度模型:"), new QLabel(viscosityModelText(material.viscosityModel), parent));
    form->addRow(zh(u8"启用密度:"), new QLabel(yesNoText(material.hasDensity), parent));
    form->addRow(zh(u8"密度:"), new QLabel(QString::number(material.density) + " " + material.densityUnit, parent));
    form->addRow(zh(u8"启用动力黏度:"), new QLabel(yesNoText(material.hasDynamicViscosity), parent));
    form->addRow(
        zh(u8"动力黏度:"),
        new QLabel(QString::number(material.dynamicViscosity) + " " + material.dynamicViscosityUnit, parent)
    );
    form->addRow(zh(u8"启用运动黏度:"), new QLabel(yesNoText(material.hasKinematicViscosity), parent));
    form->addRow(
        zh(u8"运动黏度:"),
        new QLabel(QString::number(material.kinematicViscosity) + " " + material.kinematicViscosityUnit, parent)
    );
    appendMaterialExtraProperties(form, parent, material);
    dynamicLayout->addLayout(form);
}

void SolverPropertyView::populateBoundaryCondition(
    QWidget *parent,
    const BoundaryCondition &boundaryCondition,
    const BoundaryConditionBindingSummary &bindingSummary
)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(boundaryCondition.id, parent));
    form->addRow(zh(u8"类型:"), new QLabel(boundaryConditionTypeText(boundaryCondition.type), parent));
    form->addRow(zh(u8"目标类型:"), new QLabel(boundaryTargetKindText(boundaryCondition.target.kind), parent));
    form->addRow(zh(u8"几何:"), new QLabel(boundaryCondition.target.geometryName, parent));
    form->addRow(zh(u8"面组 ID:"), new QLabel(boundaryCondition.target.faceGroupId, parent));
    form->addRow(zh(u8"面组:"), new QLabel(boundaryCondition.target.faceGroupName, parent));
    form->addRow(zh(u8"网格边界:"), new QLabel(boundaryCondition.target.meshBoundaryName, parent));
    form->addRow(zh(u8"材料 ID:"), new QLabel(boundaryCondition.materialId, parent));
    form->addRow(zh(u8"启用:"), new QLabel(yesNoText(boundaryCondition.enabled), parent));
    dynamicLayout->addLayout(form);

    auto *bindingForm = new QFormLayout;
    bindingForm->addRow(zh(u8"绑定状态:"), valueLabel(bindingSummary.status, parent));
    bindingForm->addRow(
        zh(u8"目标面组:"),
        valueLabel(bindingSummary.faceGroupDisplayName.trimmed().isEmpty() ? "-" : bindingSummary.faceGroupDisplayName, parent)
    );
    bindingForm->addRow(zh(u8"面数量:"), valueLabel(QString::number(bindingSummary.faceCount), parent));
    bindingForm->addRow(zh(u8"载荷引用:"), valueLabel(listText(bindingSummary.loads), parent));
    dynamicLayout->addLayout(bindingForm);

    for (const QString &warning : bindingSummary.warnings) {
        dynamicLayout->addWidget(warningLabel(parent, warning));
    }
}

void SolverPropertyView::populateLoad(QWidget *parent, const Load &load)
{
    auto *dynamicLayout = new QVBoxLayout(parent);
    auto *form = new QFormLayout;
    form->addRow("ID:", new QLabel(load.id, parent));
    form->addRow(zh(u8"类型:"), new QLabel(loadTypeText(load.type), parent));
    form->addRow(zh(u8"边界条件 ID:"), new QLabel(load.boundaryConditionId, parent));
    form->addRow(zh(u8"场名称:"), new QLabel(load.fieldName, parent));
    form->addRow(zh(u8"数值类型:"), new QLabel(loadValueKindText(load.value.kind), parent));
    form->addRow(zh(u8"数值:"), new QLabel(loadValueText(load), parent));
    form->addRow(zh(u8"启用:"), new QLabel(yesNoText(load.enabled), parent));
    dynamicLayout->addLayout(form);
}

void SolverPropertyView::populateSolverCategory(QWidget *parent, const SimulationCase &simulationCase)
{
    auto *dynamicLayout = new QVBoxLayout(parent);

    auto *form = new QFormLayout;
    form->addRow(zh(u8"工况 ID:"), new QLabel(simulationCase.id, parent));
    form->addRow(zh(u8"源几何:"), new QLabel(simulationCase.sourceGeometryName, parent));
    form->addRow(zh(u8"网格:"), new QLabel(simulationCase.meshName, parent));
    form->addRow(zh(u8"求解器类型:"), new QLabel(toString(simulationCase.solverType), parent));
    form->addRow(zh(u8"湍流模型:"), new QLabel(toString(simulationCase.turbulenceModel), parent));
    form->addRow(zh(u8"结束时间:"), new QLabel(QString::number(simulationCase.runControl.endTime), parent));
    form->addRow(zh(u8"时间步长:"), new QLabel(QString::number(simulationCase.runControl.timeStep), parent));
    form->addRow(zh(u8"写出间隔:"), new QLabel(QString::number(simulationCase.runControl.writeInterval), parent));
    form->addRow(zh(u8"后处理工具:"), new QLabel(simulationCase.postProcessingTool, parent));
    form->addRow(zh(u8"结构材料:"), new QLabel(QString::number(simulationCase.structuralCase.materials.size()), parent));
    form->addRow(zh(u8"结构约束/目标:"), new QLabel(QString::number(simulationCase.structuralCase.constraints.size()), parent));
    form->addRow(zh(u8"结构载荷:"), new QLabel(QString::number(simulationCase.structuralCase.loads.size()), parent));
    form->addRow(zh(u8"流体材料:"), new QLabel(QString::number(simulationCase.cfdCase.materials.size()), parent));
    form->addRow(zh(u8"CFD 边界:"), new QLabel(QString::number(simulationCase.cfdCase.boundaries.size()), parent));
    form->addRow(zh(u8"CFD 场值:"), new QLabel(QString::number(simulationCase.cfdCase.fieldValues.size()), parent));
    dynamicLayout->addLayout(form);

    dynamicLayout->addWidget(new QLabel(zh(u8"<i>仿真导出会使用当前工程模型。</i>"), parent));
}
