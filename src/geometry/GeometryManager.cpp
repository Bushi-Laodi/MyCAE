#include "GeometryManager.h"

#include "occ/OCCBooleanBuilder.h"
#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStringList>

#include <algorithm>
#include <exception>
#include <cmath>

namespace
{
constexpr double DegreesToRadians = 3.14159265358979323846 / 180.0;

QString absoluteProjectFilePath(const Project &project, const QString &filePath)
{
    return QFileInfo(filePath).isAbsolute()
        ? filePath
        : QDir(project.rootPath).filePath(filePath);
}

QString sanitizedFileBase(const QString &name)
{
    QString fileBase = name.trimmed().toLower();
    fileBase.replace(QRegularExpression("[^a-z0-9_]+"), "_");
    fileBase.replace(QRegularExpression("_+"), "_");
    fileBase = fileBase.trimmed();
    while (fileBase.startsWith('_')) {
        fileBase.remove(0, 1);
    }
    while (fileBase.endsWith('_')) {
        fileBase.chop(1);
    }
    return fileBase.isEmpty() ? "geometry" : fileBase;
}

bool loadGeometryShape(const Project &project, const GeometryObject &geometry, TopoDS_Shape *shape, QString *errorMessage)
{
    if (!shape) {
        if (errorMessage) {
            *errorMessage = "Internal error: shape output is null.";
        }
        return false;
    }

    OCCShapeIO shapeIO;
    QString loadError;
    if (!geometry.brepFile.isEmpty()) {
        const QString brepPath = absoluteProjectFilePath(project, geometry.brepFile);
        if (QFileInfo::exists(brepPath) && shapeIO.loadBREP(brepPath, *shape, &loadError)) {
            return true;
        }
    }

    if (!geometry.stepFile.isEmpty()) {
        const QString stepPath = absoluteProjectFilePath(project, geometry.stepFile);
        if (QFileInfo::exists(stepPath) && shapeIO.loadSTEP(stepPath, *shape, &loadError)) {
            return true;
        }
    }

    if (errorMessage) {
        *errorMessage = QString("Failed to load shape for geometry \"%1\". %2").arg(geometry.name, loadError);
    }
    return false;
}

bool removeProjectFileIfExists(
    const Project &project,
    const QString &filePath,
    QStringList *deletedFiles,
    QString *errorMessage
)
{
    if (filePath.trimmed().isEmpty()) {
        return true;
    }

    const QString absolutePath = absoluteProjectFilePath(project, filePath);
    if (!QFileInfo::exists(absolutePath)) {
        return true;
    }
    if (!QFile::remove(absolutePath)) {
        if (errorMessage) {
            *errorMessage = "Failed to delete file: " + absolutePath;
        }
        return false;
    }
    if (deletedFiles) {
        deletedFiles->append(absolutePath);
    }
    return true;
}

bool geometryNameExists(const Project &project, const QString &geometryDirPath, const QString &name)
{
    const QDir geometryDir(geometryDirPath);
    const QFileInfoList files = geometryDir.entryInfoList(QStringList{"*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (document.isObject() && document.object().value("name").toString() == name) {
            return true;
        }
    }

    Q_UNUSED(project);
    return false;
}

QString uniqueGeometryName(const Project &project, const QString &geometryDirPath, const QString &requestedBase)
{
    QString baseName = requestedBase.trimmed();
    if (baseName.isEmpty()) {
        baseName = "Imported_STEP";
    }

    QString candidate = baseName;
    int index = 1;
    while (geometryNameExists(project, geometryDirPath, candidate)) {
        candidate = QString("%1_%2").arg(baseName).arg(index++);
    }
    return candidate;
}

QJsonObject centerObject(double x, double y, double z)
{
    QJsonObject center;
    center.insert("x", x);
    center.insert("y", y);
    center.insert("z", z);
    return center;
}

void readCenterObject(const QJsonObject &object, double *x, double *y, double *z)
{
    const QJsonObject center = object.value("center").toObject();
    if (x) {
        *x = center.value("x").toDouble(0.0);
    }
    if (y) {
        *y = center.value("y").toDouble(0.0);
    }
    if (z) {
        *z = center.value("z").toDouble(0.0);
    }
}

bool shapeCenter(const TopoDS_Shape &shape, GeometryCenter *center, QString *errorMessage)
{
    if (!center) {
        if (errorMessage) {
            *errorMessage = "Internal error: center output is null.";
        }
        return false;
    }

    Bnd_Box bounds;
    BRepBndLib::Add(shape, bounds);
    if (bounds.IsVoid()) {
        if (errorMessage) {
            *errorMessage = "Cannot calculate geometry center: empty shape bounds.";
        }
        return false;
    }

    Standard_Real xmin = 0.0;
    Standard_Real ymin = 0.0;
    Standard_Real zmin = 0.0;
    Standard_Real xmax = 0.0;
    Standard_Real ymax = 0.0;
    Standard_Real zmax = 0.0;
    bounds.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    center->x = 0.5 * (xmin + xmax);
    center->y = 0.5 * (ymin + ymax);
    center->z = 0.5 * (zmin + zmax);
    return true;
}

gp_Trsf rotationTransform(const gp_Pnt &center, double degrees, const gp_Dir &axis)
{
    gp_Trsf transform;
    transform.SetRotation(gp_Ax1(center, axis), degrees * DegreesToRadians);
    return transform;
}

bool updateGenericGeometryTransformJson(
    const Project &project,
    const GeometryObject &geometry,
    const GeometryTransformParameters &parameters,
    const GeometryCenter &newCenter,
    QString *errorMessage
)
{
    const QString jsonPath = absoluteProjectFilePath(project, geometry.jsonFile);
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to open geometry JSON: " + file.errorString();
        }
        return false;
    }
    QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid geometry JSON: root is not an object.";
        }
        return false;
    }

    QJsonObject transform;
    transform.insert("center", centerObject(newCenter.x, newCenter.y, newCenter.z));
    transform.insert("translate", centerObject(parameters.translateX, parameters.translateY, parameters.translateZ));
    transform.insert("rotationDegrees", centerObject(
        parameters.rotateXDegrees,
        parameters.rotateYDegrees,
        parameters.rotateZDegrees
    ));
    transform.insert("uniformScale", parameters.uniformScale);
    transform.insert("transformedAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QJsonObject object = document.object();
    object.insert("lastTransform", transform);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write geometry JSON: " + file.errorString();
        }
        return false;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool updateGeometryJsonValue(
    const Project &project,
    const GeometryObject &geometry,
    const QString &key,
    const QJsonValue &value,
    QString *errorMessage
)
{
    const QString jsonPath = absoluteProjectFilePath(project, geometry.jsonFile);
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to open geometry JSON: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid geometry JSON: root is not an object.";
        }
        return false;
    }

    QJsonObject object = document.object();
    object.insert(key, value);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write geometry JSON: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}
}

bool GeometryManager::createDefaultBox(const Project &project, BoxGeometry *box, QString *errorMessage) const
{
    return createBox(project, BoxGeometry{}, box, errorMessage);
}

bool GeometryManager::createBox(const Project &project, const BoxGeometry &parameters, BoxGeometry *box, QString *errorMessage) const
{
    if (!box) {
        if (errorMessage) {
            *errorMessage = "内部错误：长方体输出对象为空。";
        }
        return false;
    }

    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "请先新建或打开工程，再创建几何对象。";
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "创建几何目录失败：" + geometryDirPath;
        }
        return false;
    }

    BoxGeometry createdBox;
    createdBox.name = nextBoxName(geometryDirPath);
    createdBox.centerX = parameters.centerX;
    createdBox.centerY = parameters.centerY;
    createdBox.centerZ = parameters.centerZ;
    createdBox.length = parameters.length;
    createdBox.width = parameters.width;
    createdBox.height = parameters.height;
    createdBox.unit = parameters.unit;
    createdBox.filePath = QDir(geometryDirPath).filePath(createdBox.name.toLower() + ".json");
    createdBox.occBrepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdBox.name.toLower() + ".brep")
    );
    createdBox.occStepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdBox.name.toLower() + ".step")
    );

    try {
        OCCGeometryFactory factory;
        OCCShapeIO shapeIO;
        const TopoDS_Shape shape = factory.createShape(createdBox);

        createdBox.occBrepSaved = shapeIO.saveBREP(
            shape,
            QDir(project.rootPath).filePath(createdBox.occBrepFile),
            &createdBox.occBrepErrorMessage
        );
        createdBox.occStepSaved = shapeIO.saveSTEP(
            shape,
            QDir(project.rootPath).filePath(createdBox.occStepFile),
            &createdBox.occStepErrorMessage
        );
    } catch (const std::exception &error) {
        createdBox.occBrepErrorMessage = error.what();
        createdBox.occStepErrorMessage = error.what();
    } catch (...) {
        createdBox.occBrepErrorMessage = "Unknown OCC export error.";
        createdBox.occStepErrorMessage = "Unknown OCC export error.";
    }

    if (!writeBoxFile(createdBox, errorMessage)) {
        return false;
    }

    *box = createdBox;
    return true;
}

bool GeometryManager::loadBoxGeometries(const Project &project, QVector<BoxGeometry> *boxes, QString *errorMessage) const
{
    if (!boxes) {
        if (errorMessage) {
            *errorMessage = "内部错误：几何输出对象为空。";
        }
        return false;
    }

    boxes->clear();

    const QDir geometryDir(geometryDirectory(project));
    if (!geometryDir.exists()) {
        return true;
    }

    const QFileInfoList files = geometryDir.entryInfoList(QStringList{"box_*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        BoxGeometry box;
        if (!readBoxFile(fileInfo.absoluteFilePath(), &box, errorMessage)) {
            return false;
        }
        boxes->append(box);
    }

    return true;
}

bool GeometryManager::createCylinder(const Project &project, const CylinderGeometry &parameters, CylinderGeometry *cylinder, QString *errorMessage) const
{
    if (!cylinder) {
        if (errorMessage) {
            *errorMessage = "Internal error: cylinder output object is null.";
        }
        return false;
    }

    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before creating geometry.";
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create geometry directory: " + geometryDirPath;
        }
        return false;
    }

    CylinderGeometry createdCylinder;
    createdCylinder.name = nextCylinderName(geometryDirPath);
    createdCylinder.centerX = parameters.centerX;
    createdCylinder.centerY = parameters.centerY;
    createdCylinder.centerZ = parameters.centerZ;
    createdCylinder.radius = parameters.radius;
    createdCylinder.height = parameters.height;
    createdCylinder.unit = parameters.unit;
    createdCylinder.filePath = QDir(geometryDirPath).filePath(createdCylinder.name.toLower() + ".json");
    createdCylinder.occBrepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdCylinder.name.toLower() + ".brep")
    );
    createdCylinder.occStepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdCylinder.name.toLower() + ".step")
    );

    try {
        OCCGeometryFactory factory;
        OCCShapeIO shapeIO;
        const TopoDS_Shape shape = factory.createShape(createdCylinder);

        createdCylinder.occBrepSaved = shapeIO.saveBREP(
            shape,
            QDir(project.rootPath).filePath(createdCylinder.occBrepFile),
            &createdCylinder.occBrepErrorMessage
        );
        createdCylinder.occStepSaved = shapeIO.saveSTEP(
            shape,
            QDir(project.rootPath).filePath(createdCylinder.occStepFile),
            &createdCylinder.occStepErrorMessage
        );
    } catch (const std::exception &error) {
        createdCylinder.occBrepErrorMessage = error.what();
        createdCylinder.occStepErrorMessage = error.what();
    } catch (...) {
        createdCylinder.occBrepErrorMessage = "Unknown OCC export error.";
        createdCylinder.occStepErrorMessage = "Unknown OCC export error.";
    }

    if (!writeCylinderFile(createdCylinder, errorMessage)) {
        return false;
    }

    *cylinder = createdCylinder;
    return true;
}

bool GeometryManager::createSphere(
    const Project &project,
    const SphereGeometry &parameters,
    SphereGeometry *sphere,
    QString *errorMessage
) const
{
    if (!sphere) {
        if (errorMessage) {
            *errorMessage = "Internal error: sphere output object is null.";
        }
        return false;
    }

    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before creating geometry.";
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create geometry directory: " + geometryDirPath;
        }
        return false;
    }

    SphereGeometry createdSphere;
    createdSphere.name = nextSphereName(geometryDirPath);
    createdSphere.centerX = parameters.centerX;
    createdSphere.centerY = parameters.centerY;
    createdSphere.centerZ = parameters.centerZ;
    createdSphere.radius = parameters.radius;
    createdSphere.unit = parameters.unit;
    createdSphere.filePath = QDir(geometryDirPath).filePath(createdSphere.name.toLower() + ".json");
    createdSphere.occBrepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdSphere.name.toLower() + ".brep")
    );
    createdSphere.occStepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(createdSphere.name.toLower() + ".step")
    );

    try {
        OCCGeometryFactory factory;
        OCCShapeIO shapeIO;
        const TopoDS_Shape shape = factory.createShape(createdSphere);

        createdSphere.occBrepSaved = shapeIO.saveBREP(
            shape,
            QDir(project.rootPath).filePath(createdSphere.occBrepFile),
            &createdSphere.occBrepErrorMessage
        );
        createdSphere.occStepSaved = shapeIO.saveSTEP(
            shape,
            QDir(project.rootPath).filePath(createdSphere.occStepFile),
            &createdSphere.occStepErrorMessage
        );
    } catch (const std::exception &error) {
        createdSphere.occBrepErrorMessage = error.what();
        createdSphere.occStepErrorMessage = error.what();
    } catch (...) {
        createdSphere.occBrepErrorMessage = "Unknown OCC export error.";
        createdSphere.occStepErrorMessage = "Unknown OCC export error.";
    }

    if (!writeSphereFile(createdSphere, errorMessage)) {
        return false;
    }

    *sphere = createdSphere;
    return true;
}

bool GeometryManager::createPlateWithHole(
    const Project &project,
    const PlateWithHoleGeometry &parameters,
    PlateWithHoleGeometry *plate,
    QString *errorMessage
) const
{
    if (!plate) {
        if (errorMessage) {
            *errorMessage = "Internal error: plate with hole output object is null.";
        }
        return false;
    }

    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before creating geometry.";
        }
        return false;
    }
    if (parameters.length <= 0.0
            || parameters.width <= 0.0
            || parameters.thickness <= 0.0
            || parameters.holeRadius <= 0.0
            || parameters.holeRadius >= std::min(parameters.length, parameters.width) * 0.5) {
        if (errorMessage) {
            *errorMessage = "Invalid plate with hole dimensions.";
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create geometry directory: " + geometryDirPath;
        }
        return false;
    }

    PlateWithHoleGeometry createdPlate;
    createdPlate.name = nextPlateWithHoleName(geometryDirPath);
    createdPlate.length = parameters.length;
    createdPlate.width = parameters.width;
    createdPlate.thickness = parameters.thickness;
    createdPlate.holeRadius = parameters.holeRadius;
    createdPlate.centerX = parameters.centerX;
    createdPlate.centerY = parameters.centerY;
    createdPlate.centerZ = parameters.centerZ;
    createdPlate.unit = parameters.unit;
    QString plateFileBase = createdPlate.name.toLower();
    plateFileBase.replace("platewithhole", "plate_with_hole");
    createdPlate.filePath = QDir(geometryDirPath).filePath(plateFileBase + ".json");
    createdPlate.occBrepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(plateFileBase + ".brep")
    );
    createdPlate.occStepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(plateFileBase + ".step")
    );

    try {
        OCCGeometryFactory factory;
        OCCShapeIO shapeIO;
        const TopoDS_Shape shape = factory.createShape(createdPlate);

        createdPlate.occBrepSaved = shapeIO.saveBREP(
            shape,
            QDir(project.rootPath).filePath(createdPlate.occBrepFile),
            &createdPlate.occBrepErrorMessage
        );
        createdPlate.occStepSaved = shapeIO.saveSTEP(
            shape,
            QDir(project.rootPath).filePath(createdPlate.occStepFile),
            &createdPlate.occStepErrorMessage
        );
    } catch (const std::exception &error) {
        createdPlate.occBrepErrorMessage = error.what();
        createdPlate.occStepErrorMessage = error.what();
    } catch (...) {
        createdPlate.occBrepErrorMessage = "Unknown OCC export error.";
        createdPlate.occStepErrorMessage = "Unknown OCC export error.";
    }

    if (!createdPlate.occBrepSaved || !createdPlate.occStepSaved) {
        if (errorMessage) {
            *errorMessage = "Failed to create plate with hole OCC files. BREP: "
                + createdPlate.occBrepErrorMessage
                + "; STEP: "
                + createdPlate.occStepErrorMessage;
        }
        return false;
    }

    if (!writePlateWithHoleFile(createdPlate, errorMessage)) {
        return false;
    }

    *plate = createdPlate;
    return true;
}

bool GeometryManager::createBooleanGeometry(
    const Project &project,
    const GeometryObject &leftGeometry,
    const GeometryObject &rightGeometry,
    GeometryBooleanOperationType operationType,
    const QString &requestedName,
    GeometryObject *geometry,
    QString *errorMessage
) const
{
    if (!geometry) {
        if (errorMessage) {
            *errorMessage = "Internal error: boolean geometry output object is null.";
        }
        return false;
    }
    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before creating geometry.";
        }
        return false;
    }
    if (leftGeometry.name == rightGeometry.name) {
        if (errorMessage) {
            *errorMessage = "Boolean operation inputs must be different geometry objects.";
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create geometry directory: " + geometryDirPath;
        }
        return false;
    }

    const QString resultName = requestedName.trimmed().isEmpty()
        ? nextBooleanName(geometryDirPath, operationType)
        : requestedName.trimmed();
    if (geometryNameExists(project, geometryDirPath, resultName)) {
        if (errorMessage) {
            *errorMessage = "Geometry name already exists: " + resultName;
        }
        return false;
    }

    const QString fileBase = sanitizedFileBase(resultName);
    const QString jsonPath = QDir(geometryDirPath).filePath(fileBase + ".json");
    if (QFileInfo::exists(jsonPath)) {
        if (errorMessage) {
            *errorMessage = "Geometry file already exists: " + jsonPath;
        }
        return false;
    }

    TopoDS_Shape leftShape;
    QString loadError;
    if (!loadGeometryShape(project, leftGeometry, &leftShape, &loadError)) {
        if (errorMessage) {
            *errorMessage = loadError;
        }
        return false;
    }

    TopoDS_Shape rightShape;
    if (!loadGeometryShape(project, rightGeometry, &rightShape, &loadError)) {
        if (errorMessage) {
            *errorMessage = loadError;
        }
        return false;
    }

    TopoDS_Shape resultShape;
    OCCBooleanBuilder booleanBuilder;
    if (!booleanBuilder.build(leftShape, rightShape, operationType, &resultShape, errorMessage)) {
        return false;
    }

    GeometryObject createdGeometry;
    createdGeometry.name = resultName;
    createdGeometry.type = "boolean";
    createdGeometry.jsonFile = QDir(project.rootPath).relativeFilePath(jsonPath);
    createdGeometry.brepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(fileBase + ".brep")
    );
    createdGeometry.stepFile = QDir(project.rootPath).relativeFilePath(
        QDir(geometryDirPath).filePath(fileBase + ".step")
    );

    OCCShapeIO shapeIO;
    if (!shapeIO.saveBREP(resultShape, absoluteProjectFilePath(project, createdGeometry.brepFile), errorMessage)) {
        return false;
    }
    if (!shapeIO.saveSTEP(resultShape, absoluteProjectFilePath(project, createdGeometry.stepFile), errorMessage)) {
        return false;
    }
    if (!writeBooleanGeometryFile(project, createdGeometry, leftGeometry, rightGeometry, operationType, errorMessage)) {
        return false;
    }

    *geometry = createdGeometry;
    return true;
}

bool GeometryManager::importStepGeometry(
    const Project &project,
    const QString &sourceStepFile,
    GeometryObject *geometry,
    QString *errorMessage
) const
{
    if (!geometry) {
        if (errorMessage) {
            *errorMessage = "Internal error: imported geometry output object is null.";
        }
        return false;
    }
    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before importing STEP.";
        }
        return false;
    }

    const QFileInfo sourceInfo(sourceStepFile);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (errorMessage) {
            *errorMessage = "STEP file does not exist: " + sourceStepFile;
        }
        return false;
    }

    const QString geometryDirPath = geometryDirectory(project);
    if (!QDir().mkpath(geometryDirPath)) {
        if (errorMessage) {
            *errorMessage = "Failed to create geometry directory: " + geometryDirPath;
        }
        return false;
    }

    OCCShapeIO shapeIO;
    TopoDS_Shape shape;
    if (!shapeIO.loadSTEP(sourceInfo.absoluteFilePath(), shape, errorMessage)) {
        return false;
    }

    const QString geometryName = uniqueGeometryName(project, geometryDirPath, sourceInfo.completeBaseName());
    const QString fileBase = sanitizedFileBase(geometryName);

    GeometryObject imported;
    imported.name = geometryName;
    imported.type = "step";
    imported.jsonFile = QDir(project.rootPath).relativeFilePath(QDir(geometryDirPath).filePath(fileBase + ".json"));
    imported.brepFile = QDir(project.rootPath).relativeFilePath(QDir(geometryDirPath).filePath(fileBase + ".brep"));
    imported.stepFile = QDir(project.rootPath).relativeFilePath(QDir(geometryDirPath).filePath(fileBase + ".step"));

    if (!shapeIO.saveBREP(shape, absoluteProjectFilePath(project, imported.brepFile), errorMessage)) {
        return false;
    }
    if (!shapeIO.saveSTEP(shape, absoluteProjectFilePath(project, imported.stepFile), errorMessage)) {
        return false;
    }
    if (!writeImportedStepGeometryFile(project, imported, sourceInfo.absoluteFilePath(), errorMessage)) {
        return false;
    }

    *geometry = imported;
    return true;
}

bool GeometryManager::deleteGeometry(
    const Project &project,
    const GeometryObject &geometry,
    QStringList *deletedFiles,
    QString *errorMessage
) const
{
    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before deleting geometry.";
        }
        return false;
    }
    if (geometry.name.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Geometry name is empty.";
        }
        return false;
    }

    if (!removeProjectFileIfExists(project, geometry.jsonFile, deletedFiles, errorMessage)) {
        return false;
    }
    if (!removeProjectFileIfExists(project, geometry.brepFile, deletedFiles, errorMessage)) {
        return false;
    }
    if (!removeProjectFileIfExists(project, geometry.stepFile, deletedFiles, errorMessage)) {
        return false;
    }

    return true;
}

bool GeometryManager::setGeometryVisible(
    const Project &project,
    const GeometryObject &geometry,
    bool visible,
    QString *errorMessage
) const
{
    return updateGeometryJsonValue(project, geometry, "visible", visible, errorMessage);
}

bool GeometryManager::geometryCenter(
    const Project &project,
    const GeometryObject &geometry,
    GeometryCenter *center,
    QString *errorMessage
) const
{
    TopoDS_Shape shape;
    if (!loadGeometryShape(project, geometry, &shape, errorMessage)) {
        return false;
    }
    return shapeCenter(shape, center, errorMessage);
}

bool GeometryManager::transformGeometry(
    const Project &project,
    const GeometryObject &geometry,
    const GeometryTransformParameters &parameters,
    GeometryObject *transformedGeometry,
    QStringList *logMessages,
    QString *errorMessage
) const
{
    if (!transformedGeometry) {
        if (errorMessage) {
            *errorMessage = "Internal error: transformed geometry output object is null.";
        }
        return false;
    }
    if (project.rootPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Please create or open a project before transforming geometry.";
        }
        return false;
    }
    if (geometry.name.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Geometry name is empty.";
        }
        return false;
    }
    if (parameters.uniformScale <= 0.0) {
        if (errorMessage) {
            *errorMessage = "Uniform scale must be greater than 0.";
        }
        return false;
    }

    TopoDS_Shape shape;
    if (!loadGeometryShape(project, geometry, &shape, errorMessage)) {
        return false;
    }

    GeometryCenter originalCenter;
    if (!shapeCenter(shape, &originalCenter, errorMessage)) {
        return false;
    }

    double dx = parameters.translateX;
    double dy = parameters.translateY;
    double dz = parameters.translateZ;
    if (parameters.setAbsoluteCenter) {
        dx += parameters.targetCenterX - originalCenter.x;
        dy += parameters.targetCenterY - originalCenter.y;
        dz += parameters.targetCenterZ - originalCenter.z;
    }

    const gp_Pnt centerPoint(originalCenter.x, originalCenter.y, originalCenter.z);
    gp_Trsf scale;
    scale.SetScale(centerPoint, parameters.uniformScale);
    const gp_Trsf rotateX = rotationTransform(centerPoint, parameters.rotateXDegrees, gp_Dir(1.0, 0.0, 0.0));
    const gp_Trsf rotateY = rotationTransform(centerPoint, parameters.rotateYDegrees, gp_Dir(0.0, 1.0, 0.0));
    const gp_Trsf rotateZ = rotationTransform(centerPoint, parameters.rotateZDegrees, gp_Dir(0.0, 0.0, 1.0));
    gp_Trsf translate;
    translate.SetTranslation(gp_Vec(dx, dy, dz));

    gp_Trsf transform;
    transform.Multiply(translate);
    transform.Multiply(rotateZ);
    transform.Multiply(rotateY);
    transform.Multiply(rotateX);
    transform.Multiply(scale);

    BRepBuilderAPI_Transform transformer(shape, transform, true);
    const TopoDS_Shape transformedShape = transformer.Shape();

    OCCShapeIO shapeIO;
    if (!shapeIO.saveBREP(transformedShape, absoluteProjectFilePath(project, geometry.brepFile), errorMessage)) {
        return false;
    }
    if (!shapeIO.saveSTEP(transformedShape, absoluteProjectFilePath(project, geometry.stepFile), errorMessage)) {
        return false;
    }

    GeometryCenter newCenter;
    if (!shapeCenter(transformedShape, &newCenter, errorMessage)) {
        return false;
    }

    if (geometry.type == "box") {
        BoxGeometry box;
        if (!readBoxFile(absoluteProjectFilePath(project, geometry.jsonFile), &box, errorMessage)) {
            return false;
        }
        box.centerX = newCenter.x;
        box.centerY = newCenter.y;
        box.centerZ = newCenter.z;
        box.length *= parameters.uniformScale;
        box.width *= parameters.uniformScale;
        box.height *= parameters.uniformScale;
        if (!writeBoxFile(box, errorMessage)) {
            return false;
        }
    } else if (geometry.type == "cylinder") {
        CylinderGeometry cylinder;
        if (!readCylinderFile(absoluteProjectFilePath(project, geometry.jsonFile), &cylinder, errorMessage)) {
            return false;
        }
        cylinder.centerX = newCenter.x;
        cylinder.centerY = newCenter.y;
        cylinder.centerZ = newCenter.z;
        cylinder.radius *= parameters.uniformScale;
        cylinder.height *= parameters.uniformScale;
        if (!writeCylinderFile(cylinder, errorMessage)) {
            return false;
        }
    } else if (geometry.type == "sphere") {
        SphereGeometry sphere;
        if (!readSphereFile(absoluteProjectFilePath(project, geometry.jsonFile), &sphere, errorMessage)) {
            return false;
        }
        sphere.centerX = newCenter.x;
        sphere.centerY = newCenter.y;
        sphere.centerZ = newCenter.z;
        sphere.radius *= parameters.uniformScale;
        if (!writeSphereFile(sphere, errorMessage)) {
            return false;
        }
    } else if (!updateGenericGeometryTransformJson(project, geometry, parameters, newCenter, errorMessage)) {
        return false;
    }

    *transformedGeometry = geometry;
    if (logMessages) {
        logMessages->append(QString("Geometry transformed: %1").arg(geometry.name));
        logMessages->append(QString("New center: (%1, %2, %3).")
            .arg(newCenter.x, 0, 'g', 8)
            .arg(newCenter.y, 0, 'g', 8)
            .arg(newCenter.z, 0, 'g', 8));
    }
    return true;
}

bool GeometryManager::loadCylinderGeometries(const Project &project, QVector<CylinderGeometry> *cylinders, QString *errorMessage) const
{
    if (!cylinders) {
        if (errorMessage) {
            *errorMessage = "Internal error: cylinder output list is null.";
        }
        return false;
    }

    cylinders->clear();

    const QDir geometryDir(geometryDirectory(project));
    if (!geometryDir.exists()) {
        return true;
    }

    const QFileInfoList files = geometryDir.entryInfoList(QStringList{"cylinder_*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        CylinderGeometry cylinder;
        if (!readCylinderFile(fileInfo.absoluteFilePath(), &cylinder, errorMessage)) {
            return false;
        }
        cylinders->append(cylinder);
    }

    return true;
}

bool GeometryManager::loadSphereGeometries(const Project &project, QVector<SphereGeometry> *spheres, QString *errorMessage) const
{
    if (!spheres) {
        if (errorMessage) {
            *errorMessage = "Internal error: sphere output list is null.";
        }
        return false;
    }

    spheres->clear();

    const QDir geometryDir(geometryDirectory(project));
    if (!geometryDir.exists()) {
        return true;
    }

    const QFileInfoList files = geometryDir.entryInfoList(QStringList{"sphere_*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        SphereGeometry sphere;
        if (!readSphereFile(fileInfo.absoluteFilePath(), &sphere, errorMessage)) {
            return false;
        }
        spheres->append(sphere);
    }

    return true;
}

bool GeometryManager::loadGeometryObjects(const Project &project, std::vector<GeometryObject> &geometries, QString *errorMessage) const
{
    geometries.clear();

    const QDir geometryDir(geometryDirectory(project));
    if (!geometryDir.exists()) {
        return true;
    }

    const QFileInfoList files = geometryDir.entryInfoList(QStringList{"*.json"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            if (errorMessage) {
                *errorMessage = "Failed to open geometry JSON: " + file.errorString();
            }
            return false;
        }

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (!document.isObject()) {
            if (errorMessage) {
                *errorMessage = "Invalid geometry JSON: root is not an object: " + fileInfo.fileName();
            }
            return false;
        }

        const QJsonObject object = document.object();
        const QString type = object.value("type").toString();
        if (type != "box"
                && type != "cylinder"
                && type != "sphere"
                && type != "plate_with_hole"
                && type != "boolean"
                && type != "step") {
            continue;
        }

        const QJsonObject occ = object.value("occ").toObject();
        GeometryObject geometry;
        geometry.name = object.value("name").toString(fileInfo.completeBaseName());
        geometry.type = type;
        geometry.jsonFile = QDir(project.rootPath).relativeFilePath(fileInfo.absoluteFilePath());
        geometry.brepFile = occ.value("brepFile").toString();
        geometry.stepFile = occ.value("stepFile").toString();
        geometry.visible = object.value("visible").toBool(true);
        if (geometry.brepFile.isEmpty()) {
            geometry.brepFile = QDir("geometry").filePath(fileInfo.completeBaseName() + ".brep");
        }
        if (geometry.stepFile.isEmpty()) {
            geometry.stepFile = QDir("geometry").filePath(fileInfo.completeBaseName() + ".step");
        }
        geometries.push_back(geometry);
    }

    return true;
}

QString GeometryManager::geometryDirectory(const Project &project) const
{
    return QDir(project.rootPath).filePath("geometry");
}

QString GeometryManager::nextBoxName(const QString &geometryDirPath) const
{
    const QDir geometryDir(geometryDirPath);
    int index = 1;
    while (QFileInfo::exists(geometryDir.filePath(QString("box_%1.json").arg(index)))) {
        ++index;
    }
    return QString("Box_%1").arg(index);
}

QString GeometryManager::nextCylinderName(const QString &geometryDirPath) const
{
    const QDir geometryDir(geometryDirPath);
    int index = 1;
    while (QFileInfo::exists(geometryDir.filePath(QString("cylinder_%1.json").arg(index)))) {
        ++index;
    }
    return QString("Cylinder_%1").arg(index);
}

QString GeometryManager::nextSphereName(const QString &geometryDirPath) const
{
    const QDir geometryDir(geometryDirPath);
    int index = 1;
    while (QFileInfo::exists(geometryDir.filePath(QString("sphere_%1.json").arg(index)))) {
        ++index;
    }
    return QString("Sphere_%1").arg(index);
}

QString GeometryManager::nextPlateWithHoleName(const QString &geometryDirPath) const
{
    const QDir geometryDir(geometryDirPath);
    int index = 1;
    while (QFileInfo::exists(geometryDir.filePath(QString("plate_with_hole_%1.json").arg(index)))) {
        ++index;
    }
    return QString("PlateWithHole_%1").arg(index);
}

QString GeometryManager::nextBooleanName(
    const QString &geometryDirPath,
    GeometryBooleanOperationType operationType
) const
{
    const QDir geometryDir(geometryDirPath);
    const QString prefix = toDisplayString(operationType);
    int index = 1;
    while (QFileInfo::exists(geometryDir.filePath(QString("%1_%2.json").arg(prefix.toLower()).arg(index)))) {
        ++index;
    }
    return QString("%1_%2").arg(prefix).arg(index);
}

bool GeometryManager::writeBoxFile(const BoxGeometry &box, QString *errorMessage) const
{
    QJsonObject dimensions;
    dimensions.insert("length", box.length);
    dimensions.insert("width", box.width);
    dimensions.insert("height", box.height);
    dimensions.insert("unit", box.unit);

    QJsonObject occ;
    occ.insert("brepFile", box.occBrepFile);
    occ.insert("stepFile", box.occStepFile);

    QJsonObject object;
    object.insert("type", "box");
    object.insert("name", box.name);
    object.insert("visible", true);
    object.insert("center", centerObject(box.centerX, box.centerY, box.centerZ));
    object.insert("dimensions", dimensions);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(box.filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "写入长方体文件失败：" + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool GeometryManager::readBoxFile(const QString &filePath, BoxGeometry *box, QString *errorMessage) const
{
    if (!box) {
        if (errorMessage) {
            *errorMessage = "内部错误：长方体输出对象为空。";
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "打开长方体文件失败：" + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "长方体文件无效：根节点不是 JSON 对象。";
        }
        return false;
    }

    const QJsonObject object = document.object();
    const QJsonObject dimensions = object.value("dimensions").toObject();
    const QJsonObject occ = object.value("occ").toObject();

    BoxGeometry loadedBox;
    loadedBox.name = object.value("name").toString(QFileInfo(filePath).baseName());
    loadedBox.length = dimensions.value("length").toDouble(200.0);
    loadedBox.width = dimensions.value("width").toDouble(200.0);
    loadedBox.height = dimensions.value("height").toDouble(200.0);
    if (object.contains("center")) {
        readCenterObject(object, &loadedBox.centerX, &loadedBox.centerY, &loadedBox.centerZ);
    } else {
        loadedBox.centerX = loadedBox.length * 0.5;
        loadedBox.centerY = loadedBox.width * 0.5;
        loadedBox.centerZ = loadedBox.height * 0.5;
    }
    loadedBox.unit = dimensions.value("unit").toString("mm");
    loadedBox.filePath = QFileInfo(filePath).absoluteFilePath();
    loadedBox.occBrepFile = occ.value("brepFile").toString();
    loadedBox.occStepFile = occ.value("stepFile").toString();
    if (loadedBox.occBrepFile.isEmpty()) {
        loadedBox.occBrepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".brep");
    }
    if (loadedBox.occStepFile.isEmpty()) {
        loadedBox.occStepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".step");
    }

    *box = loadedBox;
    return true;
}

bool GeometryManager::writeCylinderFile(const CylinderGeometry &cylinder, QString *errorMessage) const
{
    QJsonObject dimensions;
    dimensions.insert("radius", cylinder.radius);
    dimensions.insert("height", cylinder.height);
    dimensions.insert("unit", cylinder.unit);

    QJsonObject occ;
    occ.insert("brepFile", cylinder.occBrepFile);
    occ.insert("stepFile", cylinder.occStepFile);

    QJsonObject object;
    object.insert("type", "cylinder");
    object.insert("name", cylinder.name);
    object.insert("visible", true);
    object.insert("center", centerObject(cylinder.centerX, cylinder.centerY, cylinder.centerZ));
    object.insert("dimensions", dimensions);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(cylinder.filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write cylinder file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool GeometryManager::readCylinderFile(const QString &filePath, CylinderGeometry *cylinder, QString *errorMessage) const
{
    if (!cylinder) {
        if (errorMessage) {
            *errorMessage = "Internal error: cylinder output object is null.";
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to open cylinder file: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid cylinder file: root is not a JSON object.";
        }
        return false;
    }

    const QJsonObject object = document.object();
    const QJsonObject dimensions = object.value("dimensions").toObject();
    const QJsonObject occ = object.value("occ").toObject();

    CylinderGeometry loadedCylinder;
    loadedCylinder.name = object.value("name").toString(QFileInfo(filePath).baseName());
    loadedCylinder.radius = dimensions.value("radius").toDouble(50.0);
    loadedCylinder.height = dimensions.value("height").toDouble(200.0);
    if (object.contains("center")) {
        readCenterObject(object, &loadedCylinder.centerX, &loadedCylinder.centerY, &loadedCylinder.centerZ);
    } else {
        loadedCylinder.centerX = 0.0;
        loadedCylinder.centerY = 0.0;
        loadedCylinder.centerZ = loadedCylinder.height * 0.5;
    }
    loadedCylinder.unit = dimensions.value("unit").toString("mm");
    loadedCylinder.filePath = QFileInfo(filePath).absoluteFilePath();
    loadedCylinder.occBrepFile = occ.value("brepFile").toString();
    loadedCylinder.occStepFile = occ.value("stepFile").toString();
    if (loadedCylinder.occBrepFile.isEmpty()) {
        loadedCylinder.occBrepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".brep");
    }
    if (loadedCylinder.occStepFile.isEmpty()) {
        loadedCylinder.occStepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".step");
    }

    *cylinder = loadedCylinder;
    return true;
}

bool GeometryManager::writeSphereFile(const SphereGeometry &sphere, QString *errorMessage) const
{
    QJsonObject dimensions;
    dimensions.insert("radius", sphere.radius);
    dimensions.insert("unit", sphere.unit);

    QJsonObject occ;
    occ.insert("brepFile", sphere.occBrepFile);
    occ.insert("stepFile", sphere.occStepFile);

    QJsonObject object;
    object.insert("type", "sphere");
    object.insert("name", sphere.name);
    object.insert("visible", true);
    object.insert("center", centerObject(sphere.centerX, sphere.centerY, sphere.centerZ));
    object.insert("dimensions", dimensions);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(sphere.filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write sphere file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool GeometryManager::readSphereFile(const QString &filePath, SphereGeometry *sphere, QString *errorMessage) const
{
    if (!sphere) {
        if (errorMessage) {
            *errorMessage = "Internal error: sphere output object is null.";
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Failed to open sphere file: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Invalid sphere file: root is not a JSON object.";
        }
        return false;
    }

    const QJsonObject object = document.object();
    const QJsonObject dimensions = object.value("dimensions").toObject();
    const QJsonObject occ = object.value("occ").toObject();

    SphereGeometry loadedSphere;
    loadedSphere.name = object.value("name").toString(QFileInfo(filePath).baseName());
    readCenterObject(object, &loadedSphere.centerX, &loadedSphere.centerY, &loadedSphere.centerZ);
    loadedSphere.radius = dimensions.value("radius").toDouble(50.0);
    loadedSphere.unit = dimensions.value("unit").toString("mm");
    loadedSphere.filePath = QFileInfo(filePath).absoluteFilePath();
    loadedSphere.occBrepFile = occ.value("brepFile").toString();
    loadedSphere.occStepFile = occ.value("stepFile").toString();
    if (loadedSphere.occBrepFile.isEmpty()) {
        loadedSphere.occBrepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".brep");
    }
    if (loadedSphere.occStepFile.isEmpty()) {
        loadedSphere.occStepFile = QDir("geometry").filePath(QFileInfo(filePath).completeBaseName() + ".step");
    }

    *sphere = loadedSphere;
    return true;
}

bool GeometryManager::writePlateWithHoleFile(const PlateWithHoleGeometry &plate, QString *errorMessage) const
{
    QJsonObject dimensions;
    dimensions.insert("length", plate.length);
    dimensions.insert("width", plate.width);
    dimensions.insert("thickness", plate.thickness);
    dimensions.insert("holeRadius", plate.holeRadius);
    dimensions.insert("unit", plate.unit);

    QJsonObject occ;
    occ.insert("brepFile", plate.occBrepFile);
    occ.insert("stepFile", plate.occStepFile);

    QJsonObject object;
    object.insert("type", "plate_with_hole");
    object.insert("name", plate.name);
    object.insert("visible", true);
    object.insert("center", centerObject(plate.centerX, plate.centerY, plate.centerZ));
    object.insert("dimensions", dimensions);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(plate.filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write plate with hole file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool GeometryManager::writeBooleanGeometryFile(
    const Project &project,
    const GeometryObject &geometry,
    const GeometryObject &leftGeometry,
    const GeometryObject &rightGeometry,
    GeometryBooleanOperationType operationType,
    QString *errorMessage
) const
{
    QJsonArray operands;
    operands.append(leftGeometry.name);
    operands.append(rightGeometry.name);

    QJsonObject operation;
    operation.insert("type", toStorageString(operationType));
    operation.insert("leftGeometry", leftGeometry.name);
    operation.insert("rightGeometry", rightGeometry.name);
    operation.insert("operands", operands);

    QJsonObject occ;
    occ.insert("brepFile", geometry.brepFile);
    occ.insert("stepFile", geometry.stepFile);

    QJsonObject object;
    object.insert("type", geometry.type);
    object.insert("name", geometry.name);
    object.insert("visible", geometry.visible);
    object.insert("operation", operation);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(absoluteProjectFilePath(project, geometry.jsonFile));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write boolean geometry file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

bool GeometryManager::writeImportedStepGeometryFile(
    const Project &project,
    const GeometryObject &geometry,
    const QString &sourceStepFile,
    QString *errorMessage
) const
{
    QJsonObject source;
    source.insert("importedFrom", sourceStepFile);

    QJsonObject occ;
    occ.insert("brepFile", geometry.brepFile);
    occ.insert("stepFile", geometry.stepFile);

    QJsonObject object;
    object.insert("type", geometry.type);
    object.insert("name", geometry.name);
    object.insert("visible", geometry.visible);
    object.insert("source", source);
    object.insert("occ", occ);
    object.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    QFile file(absoluteProjectFilePath(project, geometry.jsonFile));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Failed to write imported STEP geometry file: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}
