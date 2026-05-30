#pragma once

#include <QString>
#include <QStringList>

class ProjectModel;

class GeometryDependencyInvalidator
{
public:
    QStringList markGeometryChanged(ProjectModel &projectModel, const QString &geometryName, const QString &reason) const;
    QStringList markMeshControlsChanged(ProjectModel &projectModel, const QString &geometryName, const QString &reason) const;
};
