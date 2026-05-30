#include "GeometryCreationController.h"

#include "geometry/GeometryManager.h"
#include "project/Project.h"
#include "ui/BoxDialog.h"
#include "ui/CylinderDialog.h"
#include "ui/PlateWithHoleDialog.h"
#include "ui/SphereDialog.h"

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

GeometryObject geometryObjectFromSphere(const Project &project, const SphereGeometry &sphere)
{
    GeometryObject geometry;
    geometry.name = sphere.name;
    geometry.type = "sphere";
    geometry.jsonFile = QFileInfo(sphere.filePath).isAbsolute()
        ? QDir(project.rootPath).relativeFilePath(sphere.filePath)
        : sphere.filePath;
    geometry.brepFile = sphere.occBrepFile;
    geometry.stepFile = sphere.occStepFile;
    return geometry;
}

GeometryObject geometryObjectFromPlateWithHole(const Project &project, const PlateWithHoleGeometry &plate)
{
    GeometryObject geometry;
    geometry.name = plate.name;
    geometry.type = "plate_with_hole";
    geometry.jsonFile = QFileInfo(plate.filePath).isAbsolute()
        ? QDir(project.rootPath).relativeFilePath(plate.filePath)
        : plate.filePath;
    geometry.brepFile = plate.occBrepFile;
    geometry.stepFile = plate.occStepFile;
    return geometry;
}

void appendOccSaveMessages(
    GeometryCreationResult &result,
    bool brepSaved,
    const QString &brepFile,
    const QString &brepError,
    bool stepSaved,
    const QString &stepFile,
    const QString &stepError
)
{
    result.logMessages.append(brepSaved ? "BREP saved: " + brepFile : "BREP save failed: " + brepError);
    result.logMessages.append(stepSaved ? "STEP saved: " + stepFile : "STEP save failed: " + stepError);
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
        appendOccSaveMessages(
            result,
            box.occBrepSaved,
            box.occBrepFile,
            box.occBrepErrorMessage,
            box.occStepSaved,
            box.occStepFile,
            box.occStepErrorMessage
        );
        result.logMessages.append("Box geometry created: " + box.name);
        return result;
    }

    if (type == GeometryCreateType::Sphere) {
        GeometryCreationResult result;

        if (project.rootPath.isEmpty()) {
            result.errorMessage = "Please create or open a project first.";
            result.logMessages.append("Create sphere failed: no project is open.");
            return result;
        }

        SphereDialog dialog(parent);
        if (dialog.exec() != QDialog::Accepted) {
            result.canceled = true;
            result.logMessages.append("Create sphere canceled.");
            return result;
        }

        QString errorMessage;
        SphereGeometry sphere;
        if (!m_geometryManager.createSphere(project, dialog.sphereParameters(), &sphere, &errorMessage)) {
            result.errorMessage = errorMessage;
            result.logMessages.append("Create sphere failed: " + errorMessage);
            return result;
        }

        result.success = true;
        result.geometryObject = geometryObjectFromSphere(project, sphere);
        appendOccSaveMessages(
            result,
            sphere.occBrepSaved,
            sphere.occBrepFile,
            sphere.occBrepErrorMessage,
            sphere.occStepSaved,
            sphere.occStepFile,
            sphere.occStepErrorMessage
        );
        result.logMessages.append("Sphere geometry created: " + sphere.name);
        return result;
    }

    if (type == GeometryCreateType::PlateWithHole) {
        GeometryCreationResult result;

        if (project.rootPath.isEmpty()) {
            result.errorMessage = "Please create or open a project first.";
            result.logMessages.append("Create plate with hole failed: no project is open.");
            return result;
        }

        PlateWithHoleDialog dialog(parent);
        if (dialog.exec() != QDialog::Accepted) {
            result.canceled = true;
            result.logMessages.append("Create plate with hole canceled.");
            return result;
        }

        QString errorMessage;
        PlateWithHoleGeometry plate;
        if (!m_geometryManager.createPlateWithHole(project, dialog.plateParameters(), &plate, &errorMessage)) {
            result.errorMessage = errorMessage;
            result.logMessages.append("Create plate with hole failed: " + errorMessage);
            return result;
        }

        result.success = true;
        result.geometryObject = geometryObjectFromPlateWithHole(project, plate);
        appendOccSaveMessages(
            result,
            plate.occBrepSaved,
            plate.occBrepFile,
            plate.occBrepErrorMessage,
            plate.occStepSaved,
            plate.occStepFile,
            plate.occStepErrorMessage
        );
        result.logMessages.append("Plate with hole geometry created: " + plate.name);
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
    appendOccSaveMessages(
        result,
        cylinder.occBrepSaved,
        cylinder.occBrepFile,
        cylinder.occBrepErrorMessage,
        cylinder.occStepSaved,
        cylinder.occStepFile,
        cylinder.occStepErrorMessage
    );
    result.logMessages.append("Cylinder geometry created: " + cylinder.name);
    return result;
}
