#pragma once

#include "geometry/GeometryObject.h"

#include <QStringList>

struct Project;
class ProjectModel;
class RenderView;

struct GeometryDisplayResult
{
    bool success = false;
    QStringList logMessages;
};

class GeometryDisplayController
{
public:
    GeometryDisplayResult displayGeometry(
        const ProjectModel &projectModel,
        const GeometryObject &geometry,
        RenderView *renderView
    ) const;
};
