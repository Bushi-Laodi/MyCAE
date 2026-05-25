#pragma once

#include "project/SelectionState.h"

#include <QStringList>

class ProjectModel;
class PropertyPanel;
class RenderView;

struct SelectionControllerResult
{
    bool accepted = false;
    QStringList logMessages;
};

class SelectionController
{
public:
    SelectionController(ProjectModel &projectModel, PropertyPanel *propertyPanel, RenderView *renderView);

    SelectionControllerResult apply(const Selection &selection) const;

private:
    SelectionControllerResult showGeometry(const QString &geometryName) const;
    SelectionControllerResult showMesh(const QString &meshName) const;
    SelectionControllerResult showFaceGroup(const QString &faceGroupId) const;
    SelectionControllerResult showSolverCategory(SelectionKind kind) const;

    ProjectModel &m_projectModel;
    PropertyPanel *m_propertyPanel = nullptr;
    RenderView *m_renderView = nullptr;
};
