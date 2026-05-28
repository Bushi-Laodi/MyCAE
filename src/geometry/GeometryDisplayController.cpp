#include "GeometryDisplayController.h"

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/SphereGeometry.h"
#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"
#include "project/Project.h"
#include "project/ProjectModel.h"
#include "ui/RenderGeometryItem.h"
#include "ui/RenderView.h"

#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include <QDir>
#include <QFileInfo>

#include <exception>
#include <vector>

namespace
{
QString absoluteProjectPath(const Project &project, const QString &filePath)
{
    return QFileInfo(filePath).isAbsolute()
        ? filePath
        : QDir(project.rootPath).filePath(filePath);
}

bool loadShapeFromGeometryFiles(
    const Project &project,
    const GeometryObject &geometry,
    TopoDS_Shape *shape,
    QStringList *logMessages
)
{
    OCCShapeIO shapeIO;
    QString errorMessage;
    if (!geometry.brepFile.isEmpty()) {
        const QString brepPath = absoluteProjectPath(project, geometry.brepFile);
        if (QFileInfo::exists(brepPath) && shapeIO.loadBREP(brepPath, *shape, &errorMessage)) {
            return true;
        }
        if (logMessages) {
            logMessages->append("BREP load skipped/failed for " + geometry.name + ": " + errorMessage);
        }
    }

    if (!geometry.stepFile.isEmpty()) {
        const QString stepPath = absoluteProjectPath(project, geometry.stepFile);
        if (QFileInfo::exists(stepPath) && shapeIO.loadSTEP(stepPath, *shape, &errorMessage)) {
            return true;
        }
        if (logMessages) {
            logMessages->append("STEP load skipped/failed for " + geometry.name + ": " + errorMessage);
        }
    }

    return false;
}

bool rebuildShapeFromParameters(
    const ProjectModel &projectModel,
    const GeometryObject &geometry,
    TopoDS_Shape *shape,
    QString *errorMessage
)
{
    try {
        OCCGeometryFactory factory;
        if (geometry.type == "box") {
            const BoxGeometry *box = projectModel.findBoxByName(geometry.name);
            if (!box) {
                if (errorMessage) {
                    *errorMessage = "missing Box parameters";
                }
                return false;
            }
            *shape = factory.createShape(*box);
            return true;
        }
        if (geometry.type == "cylinder") {
            const CylinderGeometry *cylinder = projectModel.findCylinderByName(geometry.name);
            if (!cylinder) {
                if (errorMessage) {
                    *errorMessage = "missing Cylinder parameters";
                }
                return false;
            }
            *shape = factory.createShape(*cylinder);
            return true;
        }
        if (geometry.type == "sphere") {
            const SphereGeometry *sphere = projectModel.findSphereByName(geometry.name);
            if (!sphere) {
                if (errorMessage) {
                    *errorMessage = "missing Sphere parameters";
                }
                return false;
            }
            *shape = factory.createShape(*sphere);
            return true;
        }
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = failure.GetMessageString();
        }
        return false;
    } catch (const std::exception &error) {
        if (errorMessage) {
            *errorMessage = error.what();
        }
        return false;
    }

    if (errorMessage) {
        *errorMessage = "no parameter fallback is available";
    }
    return false;
}
}

GeometryDisplayResult GeometryDisplayController::displayGeometry(
    const ProjectModel &projectModel,
    const GeometryObject &geometry,
    RenderView *renderView
) const
{
    GeometryDisplayResult result;
    if (!renderView) {
        result.logMessages.append("Geometry display failed: render view is not available.");
        return result;
    }

    const Project &project = projectModel.project();
    std::vector<RenderGeometryItem> sceneItems;
    bool selectedGeometryLoaded = false;
    for (const GeometryObject &sceneGeometry : projectModel.geometryObjects()) {
        if (!sceneGeometry.visible && sceneGeometry.name != geometry.name) {
            continue;
        }
        TopoDS_Shape shape;
        QString rebuildError;
        if (!loadShapeFromGeometryFiles(project, sceneGeometry, &shape, &result.logMessages)
            && !rebuildShapeFromParameters(projectModel, sceneGeometry, &shape, &rebuildError)) {
            result.logMessages.append(
                QString("Geometry display skipped: %1 (%2).").arg(sceneGeometry.name, rebuildError)
            );
            continue;
        }

        sceneItems.push_back(RenderGeometryItem{sceneGeometry.name, sceneGeometry.type, shape});
        if (sceneGeometry.name == geometry.name) {
            selectedGeometryLoaded = true;
        }
    }

    if (sceneItems.empty()) {
        result.logMessages.append("Geometry display failed: no geometry shape could be loaded.");
        return result;
    }
    if (!selectedGeometryLoaded) {
        result.logMessages.append("Geometry display failed: selected geometry could not be loaded: " + geometry.name);
        return result;
    }

    try {
        renderView->showGeometryScene(
            sceneItems,
            geometry.name,
            QString("%1 geometry object(s)").arg(sceneItems.size()),
            true
        );
        result.success = true;
    } catch (const Standard_Failure &failure) {
        result.logMessages.append(QString("Geometry display failed: %1.").arg(failure.GetMessageString()));
    } catch (const std::exception &error) {
        result.logMessages.append(QString("Geometry display failed: %1.").arg(error.what()));
    }
    return result;
}
