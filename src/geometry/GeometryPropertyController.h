#pragma once

#include "geometry/GeometryObject.h"

#include <QString>

class ProjectModel;
class PropertyPanel;

struct GeometryPropertyResult
{
    bool success = false;
    QString errorMessage;
};

class GeometryPropertyController
{
public:
    GeometryPropertyResult showGeometryProperties(
        const ProjectModel &projectModel,
        const GeometryObject &geometry,
        PropertyPanel *propertyPanel
    ) const;
};
