#pragma once

#include "geometry/FaceGroup.h"
#include "solver/BoundaryCondition.h"

#include <QUndoStack>
#include <QString>

#include <functional>
#include <vector>

class ProjectModel;

struct FaceGroupEditSnapshot
{
    std::vector<FaceGroup> faceGroups;
    std::vector<BoundaryCondition> boundaryConditions;
};

class UndoStackController
{
public:
    QUndoStack &stack();
    const QUndoStack &stack() const;

    void clear();
    void setFaceGroupRestoreCallback(std::function<void(const QString &selectionId)> callback);
    static FaceGroupEditSnapshot faceGroupEditSnapshot(const ProjectModel &projectModel);
    void pushFaceGroupSnapshot(
        ProjectModel &projectModel,
        FaceGroupEditSnapshot before,
        FaceGroupEditSnapshot after,
        const QString &text,
        const QString &beforeSelectionId,
        const QString &afterSelectionId
    );

private:
    QUndoStack m_stack;
    std::function<void(const QString &selectionId)> m_faceGroupRestoreCallback;
};
