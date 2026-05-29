#include "solver/BoundaryBindingInspector.h"

#include <QSet>

namespace
{
QString enabledSuffix(bool enabled)
{
    return enabled ? QString::fromUtf8(u8"启用") : QString::fromUtf8(u8"禁用");
}

QString displayNameWithId(const QString &name, const QString &id)
{
    if (name.trimmed().isEmpty() || name == id) {
        return id;
    }
    return QString("%1 (%2)").arg(name, id);
}

const FaceGroup *findFaceGroup(
    const std::vector<FaceGroup> &faceGroups,
    const BoundaryCondition &boundaryCondition
)
{
    for (const FaceGroup &faceGroup : faceGroups) {
        if (!boundaryCondition.target.faceGroupId.trimmed().isEmpty()
                && faceGroup.id == boundaryCondition.target.faceGroupId) {
            return &faceGroup;
        }
    }

    for (const FaceGroup &faceGroup : faceGroups) {
        if (faceGroup.geometryName == boundaryCondition.target.geometryName
                && !boundaryCondition.target.faceGroupName.trimmed().isEmpty()
                && faceGroup.name == boundaryCondition.target.faceGroupName) {
            return &faceGroup;
        }
    }
    return nullptr;
}
}

const FaceGroup *BoundaryBindingInspector::findTargetFaceGroup(
    const BoundaryCondition &boundaryCondition,
    const std::vector<FaceGroup> &faceGroups
)
{
    return findFaceGroup(faceGroups, boundaryCondition);
}

FaceGroupBindingSummary BoundaryBindingInspector::summarizeFaceGroup(
    const FaceGroup &faceGroup,
    const std::vector<BoundaryCondition> &boundaryConditions,
    const std::vector<Load> &loads
)
{
    FaceGroupBindingSummary summary;
    QSet<QString> boundaryConditionIds;

    for (const BoundaryCondition &boundaryCondition : boundaryConditions) {
        if (boundaryCondition.target.faceGroupId != faceGroup.id) {
            continue;
        }

        boundaryConditionIds.insert(boundaryCondition.id);
        summary.boundaryConditions.append(
            QString("%1，%2")
                .arg(displayNameWithId(boundaryCondition.name, boundaryCondition.id))
                .arg(enabledSuffix(boundaryCondition.enabled))
        );
    }

    for (const Load &load : loads) {
        if (!boundaryConditionIds.contains(load.boundaryConditionId)) {
            continue;
        }
        summary.loads.append(
            QString("%1，%2")
                .arg(displayNameWithId(load.name, load.id))
                .arg(enabledSuffix(load.enabled))
        );
    }

    if (faceGroup.faceIndices.empty() && faceGroup.faceReferences.empty()) {
        summary.warnings.append(QString::fromUtf8(u8"面组为空，边界条件或局部网格不会有可映射的面。"));
    }
    if (summary.boundaryConditions.isEmpty()) {
        summary.status = QString::fromUtf8(u8"未绑定边界条件");
        summary.warnings.append(QString::fromUtf8(u8"该面组当前没有被任何边界条件引用。"));
    } else {
        summary.status = QString::fromUtf8(u8"已绑定 %1 个边界条件，%2 个载荷")
            .arg(summary.boundaryConditions.size())
            .arg(summary.loads.size());
    }
    if (faceGroup.localMeshEnabled && faceGroup.localMeshSize <= 0.0) {
        summary.warnings.append(QString::fromUtf8(u8"局部网格已启用，但局部尺寸不是正数。"));
    }
    return summary;
}

BoundaryConditionBindingSummary BoundaryBindingInspector::summarizeBoundaryCondition(
    const BoundaryCondition &boundaryCondition,
    const std::vector<FaceGroup> &faceGroups,
    const std::vector<Load> &loads
)
{
    BoundaryConditionBindingSummary summary;
    const FaceGroup *faceGroup = findFaceGroup(faceGroups, boundaryCondition);
    if (!faceGroup) {
        summary.status = QString::fromUtf8(u8"面组引用失效");
        summary.warnings.append(QString::fromUtf8(u8"找不到该边界条件引用的面组，请重新选择目标面组。"));
    } else {
        summary.faceGroupExists = true;
        summary.faceGroupDisplayName = FaceGroups::displayName(*faceGroup);
        summary.faceCount = static_cast<int>(faceGroup->faceIndices.size());
        summary.faceGroupIsEmpty = faceGroup->faceIndices.empty() && faceGroup->faceReferences.empty();
        summary.status = summary.faceGroupIsEmpty
            ? QString::fromUtf8(u8"已绑定面组，但面组为空")
            : QString::fromUtf8(u8"已绑定面组");
        if (summary.faceGroupIsEmpty) {
            summary.warnings.append(QString::fromUtf8(u8"目标面组为空，求解器导出时无法生成有效边界。"));
        }
        if (faceGroup->geometryName != boundaryCondition.target.geometryName) {
            summary.warnings.append(QString::fromUtf8(u8"面组所属几何与边界条件记录的几何不一致。"));
        }
    }

    for (const Load &load : loads) {
        if (load.boundaryConditionId != boundaryCondition.id) {
            continue;
        }
        summary.loads.append(
            QString("%1，%2")
                .arg(displayNameWithId(load.name, load.id))
                .arg(enabledSuffix(load.enabled))
        );
    }

    if (summary.loads.isEmpty()) {
        summary.loads.append(QString::fromUtf8(u8"无载荷引用"));
    }
    return summary;
}
