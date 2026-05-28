#include "GeometryPropertyController.h"

#include "geometry/GeometryManager.h"
#include "geometry/FaceGroup.h"
#include "geometry/SphereGeometry.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "result/ResultObject.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "ui/PropertyPanel.h"
#include "ui/property/GeometryPropertyView.h"

namespace
{
QString vectorText(double x, double y, double z, const QString &unit = QString("mm"))
{
    return QString("(%1, %2, %3) %4")
        .arg(x, 0, 'g', 6)
        .arg(y, 0, 'g', 6)
        .arg(z, 0, 'g', 6)
        .arg(unit);
}

GeometryPropertyDetails buildDetails(const ProjectModel &projectModel, const GeometryObject &geometry)
{
    GeometryPropertyDetails details;
    details.brepFile = geometry.brepFile;
    details.stepFile = geometry.stepFile;
    details.visible = geometry.visible;

    GeometryCenter center;
    QString centerError;
    if (GeometryManager().geometryCenter(projectModel.project(), geometry, &center, &centerError)) {
        details.center = vectorText(center.x, center.y, center.z);
    }

    QStringList meshNamesForResults;
    for (const MeshObject &mesh : projectModel.meshObjects()) {
        if (mesh.sourceGeometryName != geometry.name) {
            continue;
        }
        details.meshNames.append(mesh.name);
        meshNamesForResults.append(mesh.name);
        if (mesh.stale) {
            details.staleMeshNames.append(mesh.name + (mesh.staleReason.isEmpty() ? QString() : " - " + mesh.staleReason));
        }
    }

    QStringList boundaryConditionIds;
    for (const FaceGroup &faceGroup : projectModel.faceGroups()) {
        if (faceGroup.geometryName != geometry.name) {
            continue;
        }
        const QString displayName = FaceGroups::displayName(faceGroup);
        details.faceGroupNames.append(displayName);
        if (faceGroup.needsReview) {
            details.reviewFaceGroupNames.append(displayName + (faceGroup.reviewReason.isEmpty() ? QString() : " - " + faceGroup.reviewReason));
        }
    }

    for (const BoundaryCondition &boundaryCondition : projectModel.boundaryConditions()) {
        if (boundaryCondition.target.geometryName == geometry.name) {
            details.boundaryConditionNames.append(boundaryCondition.name);
            boundaryConditionIds.append(boundaryCondition.id);
        }
    }

    for (const Load &load : projectModel.loads()) {
        if (boundaryConditionIds.contains(load.boundaryConditionId)) {
            details.loadNames.append(load.name);
        }
    }

    for (const ResultObject &result : projectModel.resultRepository().results()) {
        if (!meshNamesForResults.contains(result.meshName)) {
            continue;
        }
        details.resultNames.append(result.name);
        if (result.stale) {
            details.staleResultNames.append(result.name + (result.staleReason.isEmpty() ? QString() : " - " + result.staleReason));
        }
    }

    return details;
}
}

GeometryPropertyResult GeometryPropertyController::showGeometryProperties(
    const ProjectModel &projectModel,
    const GeometryObject &geometry,
    PropertyPanel *propertyPanel
) const
{
    GeometryPropertyResult result;
    if (!propertyPanel) {
        result.errorMessage = "Property display failed: property panel is not available.";
        return result;
    }

    if (geometry.type == "box") {
        const BoxGeometry *box = projectModel.findBoxByName(geometry.name);
        if (!box) {
            result.errorMessage = "Property display failed: missing Box parameters for " + geometry.name;
            return result;
        }
        GeometryPropertyDetails details = buildDetails(projectModel, geometry);
        details.dimensions = QString("L=%1 %4, W=%2 %4, H=%3 %4")
            .arg(box->length, 0, 'g', 6)
            .arg(box->width, 0, 'g', 6)
            .arg(box->height, 0, 'g', 6)
            .arg(box->unit);
        propertyPanel->showBoxGeometry(*box, geometry, details);
        result.success = true;
        return result;
    }

    if (geometry.type == "cylinder") {
        const CylinderGeometry *cylinder = projectModel.findCylinderByName(geometry.name);
        if (!cylinder) {
            result.errorMessage = "Property display failed: missing Cylinder parameters for " + geometry.name;
            return result;
        }
        GeometryPropertyDetails details = buildDetails(projectModel, geometry);
        details.dimensions = QString("R=%1 %3, H=%2 %3")
            .arg(cylinder->radius, 0, 'g', 6)
            .arg(cylinder->height, 0, 'g', 6)
            .arg(cylinder->unit);
        propertyPanel->showCylinderGeometry(*cylinder, geometry, details);
        result.success = true;
        return result;
    }

    if (geometry.type == "sphere") {
        const SphereGeometry *sphere = projectModel.findSphereByName(geometry.name);
        if (!sphere) {
            result.errorMessage = "Property display failed: missing Sphere parameters for " + geometry.name;
            return result;
        }
        GeometryPropertyDetails details = buildDetails(projectModel, geometry);
        details.dimensions = QString("R=%1 %2")
            .arg(sphere->radius, 0, 'g', 6)
            .arg(sphere->unit);
        propertyPanel->showSphereGeometry(*sphere, geometry, details);
        result.success = true;
        return result;
    }

    if (geometry.type == "boolean" || geometry.type == "step") {
        propertyPanel->showGeometryObject(geometry, buildDetails(projectModel, geometry));
        result.success = true;
        return result;
    }

    result.errorMessage = "Property display failed: unsupported geometry type " + geometry.type;
    return result;
}
