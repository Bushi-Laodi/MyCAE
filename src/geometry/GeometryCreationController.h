#pragma once

#include "geometry/GeometryObject.h"

#include <QString>
#include <QStringList>

class GeometryManager;
struct Project;
class QWidget;

enum class GeometryCreateType
{
    Box,
    Cylinder
};

struct GeometryCreationResult
{
    bool success = false;
    bool canceled = false;
    GeometryObject geometryObject;
    QString errorMessage;
    QStringList logMessages;
};

class GeometryCreationController
{
public:
    explicit GeometryCreationController(const GeometryManager &geometryManager);

    GeometryCreationResult createGeometry(QWidget *parent, const Project &project, GeometryCreateType type) const;

private:
    const GeometryManager &m_geometryManager;
};
