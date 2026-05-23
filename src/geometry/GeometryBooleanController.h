#pragma once

#include "geometry/GeometryObject.h"

#include <QString>
#include <QStringList>

class GeometryManager;
class ProjectModel;
class QWidget;

struct GeometryBooleanResult
{
    bool success = false;
    bool canceled = false;
    GeometryObject geometryObject;
    QString errorMessage;
    QStringList logMessages;
};

class GeometryBooleanController
{
public:
    explicit GeometryBooleanController(const GeometryManager &geometryManager);

    GeometryBooleanResult createBooleanGeometry(QWidget *parent, const ProjectModel &projectModel) const;

private:
    const GeometryManager &m_geometryManager;
};
