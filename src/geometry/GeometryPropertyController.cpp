#include "GeometryPropertyController.h"

#include "project/ProjectModel.h"
#include "ui/PropertyPanel.h"

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
        propertyPanel->showBoxGeometry(*box);
        result.success = true;
        return result;
    }

    if (geometry.type == "cylinder") {
        const CylinderGeometry *cylinder = projectModel.findCylinderByName(geometry.name);
        if (!cylinder) {
            result.errorMessage = "Property display failed: missing Cylinder parameters for " + geometry.name;
            return result;
        }
        propertyPanel->showCylinderGeometry(*cylinder);
        result.success = true;
        return result;
    }

    if (geometry.type == "boolean") {
        propertyPanel->showGeometryObject(geometry);
        result.success = true;
        return result;
    }

    result.errorMessage = "Property display failed: unsupported geometry type " + geometry.type;
    return result;
}
