#pragma once

#include "solver/SolverDataService.h"

#include <QString>
#include <QStringList>

class ProjectModel;
class PropertyPanel;
class QWidget;

using SolverDataControllerResult = SolverDataServiceResult;

class SolverDataController
{
public:
    static QStringList showMaterialCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel);
    static QStringList showBoundaryConditionCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel);
    static QStringList showLoadCategory(ProjectModel &projectModel, PropertyPanel *propertyPanel);
    static QStringList showMaterial(ProjectModel &projectModel, PropertyPanel *propertyPanel, const QString &materialId);
    static QStringList showBoundaryCondition(
        ProjectModel &projectModel,
        PropertyPanel *propertyPanel,
        const QString &boundaryConditionId
    );
    static QStringList showLoad(ProjectModel &projectModel, PropertyPanel *propertyPanel, const QString &loadId);

    static SolverDataControllerResult createMaterial(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createStructuralMaterial(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createFluidMaterial(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createBoundaryCondition(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createStructuralBoundaryCondition(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createCfdBoundaryCondition(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createLoad(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createStructuralLoad(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult createCfdFieldValue(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult editSelected(QWidget *parent, ProjectModel &projectModel);
    static SolverDataControllerResult deleteSelected(QWidget *parent, ProjectModel &projectModel);
};
