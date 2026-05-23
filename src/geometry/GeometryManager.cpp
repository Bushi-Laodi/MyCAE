#include "GeometryManager.h"

#include "occ/OCCBooleanBuilder.h"
#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"

#include <TopoDS_Shape.hxx>

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

#include <exception>

namespace
{
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
        if (type != "box" && type != "cylinder" && type != "boolean") {
            continue;
        }

        const QJsonObject occ = object.value("occ").toObject();
        GeometryObject geometry;
        geometry.name = object.value("name").toString(fileInfo.completeBaseName());
        geometry.type = type;
        geometry.jsonFile = QDir(project.rootPath).relativeFilePath(fileInfo.absoluteFilePath());
        geometry.brepFile = occ.value("brepFile").toString();
        geometry.stepFile = occ.value("stepFile").toString();
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
