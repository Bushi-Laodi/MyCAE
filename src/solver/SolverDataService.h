#pragma once

#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"
#include "solver/SectionAssignment.h"

#include <QString>
#include <QStringList>

class ProjectModel;

enum class SolverDataSelectionKind
{
    None,
    MaterialCategory,
    SectionAssignmentCategory,
    BoundaryConditionCategory,
    LoadCategory,
    Material,
    SectionAssignment,
    BoundaryCondition,
    Load
};

struct SolverDataServiceResult
{
    bool changed = false;
    SolverDataSelectionKind selectionKind = SolverDataSelectionKind::None;
    QString selectionId;
    QStringList logMessages;
};

class SolverDataService
{
public:
    static SolverDataServiceResult createMaterial(ProjectModel &projectModel, const Material &material);
    static SolverDataServiceResult createSectionAssignment(
        ProjectModel &projectModel,
        const SectionAssignment &sectionAssignment
    );
    static SolverDataServiceResult createBoundaryCondition(
        ProjectModel &projectModel,
        const BoundaryCondition &boundaryCondition
    );
    static SolverDataServiceResult createLoad(ProjectModel &projectModel, const Load &load);
    static SolverDataServiceResult updateMaterial(
        ProjectModel &projectModel,
        const QString &originalId,
        const Material &material
    );
    static SolverDataServiceResult updateSectionAssignment(
        ProjectModel &projectModel,
        const QString &originalId,
        const SectionAssignment &sectionAssignment
    );
    static SolverDataServiceResult updateBoundaryCondition(
        ProjectModel &projectModel,
        const QString &originalId,
        const BoundaryCondition &boundaryCondition
    );
    static SolverDataServiceResult updateLoad(ProjectModel &projectModel, const QString &originalId, const Load &load);
    static SolverDataServiceResult deleteMaterial(ProjectModel &projectModel, const QString &materialId);
    static SolverDataServiceResult deleteSectionAssignment(ProjectModel &projectModel, const QString &sectionAssignmentId);
    static SolverDataServiceResult deleteBoundaryCondition(
        ProjectModel &projectModel,
        const QString &boundaryConditionId
    );
    static SolverDataServiceResult deleteLoad(ProjectModel &projectModel, const QString &loadId);
};
