#include "commands/UndoStackController.h"

#include "project/ProjectModel.h"

#include <utility>

namespace
{
bool facePointEquals(const FacePoint &left, const FacePoint &right)
{
    return left.x == right.x && left.y == right.y && left.z == right.z;
}

bool faceReferenceEquals(const FaceReference &left, const FaceReference &right)
{
    return left.faceIndex == right.faceIndex
        && facePointEquals(left.pickedPoint, right.pickedPoint)
        && facePointEquals(left.center, right.center)
        && facePointEquals(left.normal, right.normal)
        && left.area == right.area;
}

bool faceGroupEquals(const FaceGroup &left, const FaceGroup &right)
{
    if (left.id != right.id
            || left.name != right.name
            || left.geometryName != right.geometryName
            || left.role != right.role
            || left.faceIndices != right.faceIndices
            || left.physicalGroupEnabled != right.physicalGroupEnabled
            || left.localMeshEnabled != right.localMeshEnabled
            || left.localMeshSize != right.localMeshSize
            || left.faceReferences.size() != right.faceReferences.size()) {
        return false;
    }
    for (size_t i = 0; i < left.faceReferences.size(); ++i) {
        if (!faceReferenceEquals(left.faceReferences.at(i), right.faceReferences.at(i))) {
            return false;
        }
    }
    return true;
}

bool faceGroupsEqual(const std::vector<FaceGroup> &left, const std::vector<FaceGroup> &right)
{
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); ++i) {
        if (!faceGroupEquals(left.at(i), right.at(i))) {
            return false;
        }
    }
    return true;
}

bool boundaryTargetEquals(const BoundaryTarget &left, const BoundaryTarget &right)
{
    return left.kind == right.kind
        && left.geometryName == right.geometryName
        && left.faceGroupId == right.faceGroupId
        && left.faceGroupName == right.faceGroupName
        && left.meshBoundaryName == right.meshBoundaryName;
}

bool boundaryConditionEquals(const BoundaryCondition &left, const BoundaryCondition &right)
{
    return left.id == right.id
        && left.name == right.name
        && left.type == right.type
        && boundaryTargetEquals(left.target, right.target)
        && left.materialId == right.materialId
        && left.enabled == right.enabled;
}

bool boundaryConditionsEqual(
    const std::vector<BoundaryCondition> &left,
    const std::vector<BoundaryCondition> &right
)
{
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); ++i) {
        if (!boundaryConditionEquals(left.at(i), right.at(i))) {
            return false;
        }
    }
    return true;
}

bool snapshotsEqual(const FaceGroupEditSnapshot &left, const FaceGroupEditSnapshot &right)
{
    return faceGroupsEqual(left.faceGroups, right.faceGroups)
        && boundaryConditionsEqual(left.boundaryConditions, right.boundaryConditions);
}

class FaceGroupSnapshotCommand final : public QUndoCommand
{
public:
    FaceGroupSnapshotCommand(
        ProjectModel &projectModel,
        FaceGroupEditSnapshot before,
        FaceGroupEditSnapshot after,
        QString beforeSelectionId,
        QString afterSelectionId,
        std::function<void(const QString &)> callback,
        const QString &text
    )
        : QUndoCommand(text)
        , m_projectModel(projectModel)
        , m_before(std::move(before))
        , m_after(std::move(after))
        , m_beforeSelectionId(std::move(beforeSelectionId))
        , m_afterSelectionId(std::move(afterSelectionId))
        , m_callback(std::move(callback))
    {
    }

    void undo() override
    {
        restore(m_before, m_beforeSelectionId);
    }

    void redo() override
    {
        if (m_firstRedo) {
            m_firstRedo = false;
            return;
        }
        restore(m_after, m_afterSelectionId);
    }

private:
    void restore(const FaceGroupEditSnapshot &snapshot, const QString &selectionId)
    {
        m_projectModel.solverRepository().faceGroups() = snapshot.faceGroups;
        m_projectModel.solverRepository().boundaryConditions() = snapshot.boundaryConditions;
        if (m_callback) {
            m_callback(selectionId);
        }
    }

    ProjectModel &m_projectModel;
    FaceGroupEditSnapshot m_before;
    FaceGroupEditSnapshot m_after;
    QString m_beforeSelectionId;
    QString m_afterSelectionId;
    std::function<void(const QString &)> m_callback;
    bool m_firstRedo = true;
};
}

QUndoStack &UndoStackController::stack()
{
    return m_stack;
}

const QUndoStack &UndoStackController::stack() const
{
    return m_stack;
}

void UndoStackController::clear()
{
    m_stack.clear();
}

void UndoStackController::setFaceGroupRestoreCallback(std::function<void(const QString &selectionId)> callback)
{
    m_faceGroupRestoreCallback = std::move(callback);
}

FaceGroupEditSnapshot UndoStackController::faceGroupEditSnapshot(const ProjectModel &projectModel)
{
    return FaceGroupEditSnapshot{
        projectModel.solverRepository().faceGroups(),
        projectModel.solverRepository().boundaryConditions()
    };
}

void UndoStackController::pushFaceGroupSnapshot(
    ProjectModel &projectModel,
    FaceGroupEditSnapshot before,
    FaceGroupEditSnapshot after,
    const QString &text,
    const QString &beforeSelectionId,
    const QString &afterSelectionId
)
{
    if (snapshotsEqual(before, after)) {
        return;
    }
    m_stack.push(new FaceGroupSnapshotCommand(
        projectModel,
        std::move(before),
        std::move(after),
        beforeSelectionId,
        afterSelectionId,
        m_faceGroupRestoreCallback,
        text
    ));
}
