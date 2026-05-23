#include "GeometryDisplayController.h"

#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"
#include "project/Project.h"
#include "project/ProjectModel.h"
#include "ui/RenderView.h"

#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include <QDir>
#include <QFileInfo>

#include <exception>

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
    if (!geometry.brepFile.isEmpty()) {
        const QString brepPath = QFileInfo(geometry.brepFile).isAbsolute()
            ? geometry.brepFile
            : QDir(project.rootPath).filePath(geometry.brepFile);

        if (QFileInfo::exists(brepPath)) {
            QString errorMessage;
            TopoDS_Shape loadedShape;
            OCCShapeIO shapeIO;
            if (shapeIO.loadBREP(brepPath, loadedShape, &errorMessage)) {
                try {
                    renderView->showOccShape(loadedShape, geometry.name, geometry.type);
                    result.success = true;
                    return result;
                } catch (const Standard_Failure &failure) {
                    result.logMessages.append(
                        QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(failure.GetMessageString())
                    );
                } catch (const std::exception &error) {
                    result.logMessages.append(
                        QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(error.what())
                    );
                }
            } else {
                result.logMessages.append("BREP load failed: " + errorMessage + "; rebuilding OCC shape from parameters.");
            }
        } else {
            result.logMessages.append("BREP file does not exist: " + geometry.brepFile + "; rebuilding OCC shape from parameters.");
        }
    } else {
        result.logMessages.append("Geometry has no BREP file; rebuilding OCC shape from parameters.");
    }

    if (geometry.type == "box") {
        const BoxGeometry *box = projectModel.findBoxByName(geometry.name);
        if (!box) {
            result.logMessages.append("Geometry display failed: box parameters were not loaded: " + geometry.name);
            return result;
        }

        GeometryDisplayResult fallbackResult = displayBoxGeometry(project, *box, renderView);
        result.success = fallbackResult.success;
        result.logMessages.append(fallbackResult.logMessages);
        return result;
    }

    if (geometry.type == "cylinder") {
        const CylinderGeometry *cylinder = projectModel.findCylinderByName(geometry.name);
        if (!cylinder) {
            result.logMessages.append("Geometry display failed: cylinder parameters were not loaded: " + geometry.name);
            return result;
        }

        GeometryDisplayResult fallbackResult = displayCylinderGeometry(project, *cylinder, renderView);
        result.success = fallbackResult.success;
        result.logMessages.append(fallbackResult.logMessages);
        return result;
    }

    result.logMessages.append("Geometry display failed: unsupported geometry type: " + geometry.type);
    return result;
}

GeometryDisplayResult GeometryDisplayController::displayBoxGeometry(
    const Project &project,
    const BoxGeometry &box,
    RenderView *renderView
) const
{
    GeometryDisplayResult result;
    if (!renderView) {
        result.logMessages.append("Geometry display failed: render view is not available.");
        return result;
    }

    const QString sizeText = QString("%1 %4 x %2 %4 x %3 %4")
        .arg(box.length)
        .arg(box.width)
        .arg(box.height)
        .arg(box.unit);

    const QString brepPath = QFileInfo(box.occBrepFile).isAbsolute()
        ? box.occBrepFile
        : QDir(project.rootPath).filePath(box.occBrepFile);

    if (!box.occBrepFile.isEmpty() && QFileInfo::exists(brepPath)) {
        QString errorMessage;
        TopoDS_Shape loadedShape;
        OCCShapeIO shapeIO;
        if (shapeIO.loadBREP(brepPath, loadedShape, &errorMessage)) {
            try {
                renderView->showOccShape(loadedShape, box.name, sizeText);
                result.success = true;
                return result;
            } catch (const Standard_Failure &failure) {
                result.logMessages.append(
                    QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(failure.GetMessageString())
                );
            } catch (const std::exception &error) {
                result.logMessages.append(
                    QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(error.what())
                );
            }
        } else {
            result.logMessages.append("BREP load failed: " + errorMessage + "; rebuilding OCC shape from parameters.");
        }
    } else if (!box.occBrepFile.isEmpty()) {
        result.logMessages.append("BREP file does not exist: " + box.occBrepFile + "; rebuilding OCC shape from parameters.");
    }

    try {
        OCCGeometryFactory factory;
        const TopoDS_Shape shape = factory.createShape(box);
        renderView->showOccShape(shape, box.name, sizeText);
        result.success = true;
    } catch (const Standard_Failure &failure) {
        result.logMessages.append(
            QString("OCC box display failed: %1; falling back to vtkCubeSource.").arg(failure.GetMessageString())
        );
        renderView->showBoxGeometry(box);
        result.success = true;
    } catch (const std::exception &error) {
        result.logMessages.append(
            QString("OCC box display failed: %1; falling back to vtkCubeSource.").arg(error.what())
        );
        renderView->showBoxGeometry(box);
        result.success = true;
    }

    return result;
}

GeometryDisplayResult GeometryDisplayController::displayCylinderGeometry(
    const Project &project,
    const CylinderGeometry &cylinder,
    RenderView *renderView
) const
{
    GeometryDisplayResult result;
    if (!renderView) {
        result.logMessages.append("Geometry display failed: render view is not available.");
        return result;
    }

    const QString sizeText = QString("R %1 %3 x H %2 %3")
        .arg(cylinder.radius)
        .arg(cylinder.height)
        .arg(cylinder.unit);

    const QString brepPath = QFileInfo(cylinder.occBrepFile).isAbsolute()
        ? cylinder.occBrepFile
        : QDir(project.rootPath).filePath(cylinder.occBrepFile);

    if (!cylinder.occBrepFile.isEmpty() && QFileInfo::exists(brepPath)) {
        QString errorMessage;
        TopoDS_Shape loadedShape;
        OCCShapeIO shapeIO;
        if (shapeIO.loadBREP(brepPath, loadedShape, &errorMessage)) {
            try {
                renderView->showOccShape(loadedShape, cylinder.name, sizeText);
                result.success = true;
                return result;
            } catch (const Standard_Failure &failure) {
                result.logMessages.append(
                    QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(failure.GetMessageString())
                );
            } catch (const std::exception &error) {
                result.logMessages.append(
                    QString("BREP display failed: %1; rebuilding OCC shape from parameters.").arg(error.what())
                );
            }
        } else {
            result.logMessages.append("BREP load failed: " + errorMessage + "; rebuilding OCC shape from parameters.");
        }
    } else if (!cylinder.occBrepFile.isEmpty()) {
        result.logMessages.append("BREP file does not exist: " + cylinder.occBrepFile + "; rebuilding OCC shape from parameters.");
    }

    try {
        OCCGeometryFactory factory;
        const TopoDS_Shape shape = factory.createShape(cylinder);
        renderView->showOccShape(shape, cylinder.name, sizeText);
        result.success = true;
    } catch (const Standard_Failure &failure) {
        result.logMessages.append(QString("OCC cylinder display failed: %1.").arg(failure.GetMessageString()));
    } catch (const std::exception &error) {
        result.logMessages.append(QString("OCC cylinder display failed: %1.").arg(error.what()));
    }

    return result;
}
