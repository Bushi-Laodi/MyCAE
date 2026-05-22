#include "GeometryCreationController.h"

#include "geometry/GeometryManager.h"
#include "project/Project.h"
#include "ui/BoxDialog.h"
#include "ui/CylinderDialog.h"

#include <QDir>
#include <QFileInfo>

namespace
{
GeometryObject geometryObjectFromBox(const Project &project, const BoxGeometry &box)
{
    GeometryObject geometry;
    geometry.name = box.name;
    geometry.type = "box";
    geometry.jsonFile = QFileInfo(box.filePath).isAbsolute()
        ? QDir(project.rootPath).relativeFilePath(box.filePath)
        : box.filePath;
    geometry.brepFile = box.occBrepFile;
    geometry.stepFile = box.occStepFile;
    return geometry;
}

GeometryObject geometryObjectFromCylinder(const Project &project, const CylinderGeometry &cylinder)
{
    GeometryObject geometry;
    geometry.name = cylinder.name;
    geometry.type = "cylinder";
    geometry.jsonFile = QFileInfo(cylinder.filePath).isAbsolute()
        ? QDir(project.rootPath).relativeFilePath(cylinder.filePath)
        : cylinder.filePath;
    geometry.brepFile = cylinder.occBrepFile;
    geometry.stepFile = cylinder.occStepFile;
    return geometry;
}
}

GeometryCreationController::GeometryCreationController(const GeometryManager &geometryManager)
    : m_geometryManager(geometryManager)
{
}

GeometryCreationResult GeometryCreationController::createGeometry(QWidget *parent, const Project &project, GeometryCreateType type) const
{
    if (type == GeometryCreateType::Box) {
        GeometryCreationResult result;

        if (project.rootPath.isEmpty()) {
            result.errorMessage = "Please create or open a project first.";
            result.logMessages.append("Create box failed: no project is open.");
            return result;
        }

        BoxDialog dialog(parent);
        if (dialog.exec() != QDialog::Accepted) {
            result.canceled = true;
            result.logMessages.append("Create box canceled.");
            return result;
        }

        QString errorMessage;
        BoxGeometry box;
        if (!m_geometryManager.createBox(project, dialog.boxParameters(), &box, &errorMessage)) {
            result.errorMessage = errorMessage;
            result.logMessages.append("Create box failed: " + errorMessage);
            return result;
        }

        result.success = true;
        result.geometryObject = geometryObjectFromBox(project, box);
        if (box.occBrepSaved) {
            result.logMessages.append("BREP saved: " + box.occBrepFile);
        } else {
            result.logMessages.append("BREP save failed: " + box.occBrepErrorMessage);
        }
        if (box.occStepSaved) {
            result.logMessages.append("STEP saved: " + box.occStepFile);
        } else {
            result.logMessages.append("STEP save failed: " + box.occStepErrorMessage);
        }
        result.logMessages.append("Box geometry created: " + box.name);
        return result;
    }

    GeometryCreationResult result;

    if (project.rootPath.isEmpty()) {
        result.errorMessage = "Please create or open a project first.";
        result.logMessages.append("Create cylinder failed: no project is open.");
        return result;
    }

    CylinderDialog dialog(parent);
    if (dialog.exec() != QDialog::Accepted) {
        result.canceled = true;
        result.logMessages.append("Create cylinder canceled.");
        return result;
    }

    QString errorMessage;
    CylinderGeometry cylinder;
    if (!m_geometryManager.createCylinder(project, dialog.cylinderParameters(), &cylinder, &errorMessage)) {
        result.errorMessage = errorMessage;
        result.logMessages.append("Create cylinder failed: " + errorMessage);
        return result;
    }

    result.success = true;
    result.geometryObject = geometryObjectFromCylinder(project, cylinder);
    if (cylinder.occBrepSaved) {
        result.logMessages.append("BREP saved: " + cylinder.occBrepFile);
    } else {
        result.logMessages.append("BREP save failed: " + cylinder.occBrepErrorMessage);
    }
    if (cylinder.occStepSaved) {
        result.logMessages.append("STEP saved: " + cylinder.occStepFile);
    } else {
        result.logMessages.append("STEP save failed: " + cylinder.occStepErrorMessage);
    }
    result.logMessages.append("Cylinder geometry created: " + cylinder.name);
    return result;
}
