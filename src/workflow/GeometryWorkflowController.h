#pragma once

#include "geometry/GeometryCreationController.h"

#include <QStringList>

class GeometryManager;
class ProjectModel;
class ProjectWorkflowController;
class PropertyPanel;
class RenderView;
class QWidget;

struct GeometryWorkflowResult
{
    bool success = false;
    bool canceled = false;
    QStringList logMessages;
};

class GeometryWorkflowController
{
public:
    GeometryWorkflowController(
        GeometryManager &geometryManager,
        ProjectModel &projectModel,
        ProjectWorkflowController &projectWorkflow,
        PropertyPanel *propertyPanel,
        RenderView *renderView,
        QWidget *parent
    );

    GeometryWorkflowResult createGeometry(GeometryCreateType type) const;
    GeometryWorkflowResult createBooleanGeometry() const;
    GeometryWorkflowResult importStepGeometry() const;
    GeometryWorkflowResult transformSelectedGeometry() const;
    GeometryWorkflowResult deleteSelectedGeometry() const;

private:
    GeometryManager &m_geometryManager;
    ProjectModel &m_projectModel;
    ProjectWorkflowController &m_projectWorkflow;
    PropertyPanel *m_propertyPanel = nullptr;
    RenderView *m_renderView = nullptr;
    QWidget *m_parent = nullptr;
};
