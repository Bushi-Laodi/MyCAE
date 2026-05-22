#pragma once

#include "geometry/GeometryObject.h"

#include <QStringList>

struct Project;
struct BoxGeometry;
struct CylinderGeometry;
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

private:
    GeometryDisplayResult displayBoxGeometry(
        const Project &project,
        const BoxGeometry &box,
        RenderView *renderView
    ) const;

    GeometryDisplayResult displayCylinderGeometry(
        const Project &project,
        const CylinderGeometry &cylinder,
        RenderView *renderView
    ) const;
};
