#include "GeometryManager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

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

bool GeometryManager::writeBoxFile(const BoxGeometry &box, QString *errorMessage) const
{
    QJsonObject dimensions;
    dimensions.insert("length", box.length);
    dimensions.insert("width", box.width);
    dimensions.insert("height", box.height);
    dimensions.insert("unit", box.unit);

    QJsonObject object;
    object.insert("type", "box");
    object.insert("name", box.name);
    object.insert("dimensions", dimensions);
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

    BoxGeometry loadedBox;
    loadedBox.name = object.value("name").toString(QFileInfo(filePath).baseName());
    loadedBox.length = dimensions.value("length").toDouble(200.0);
    loadedBox.width = dimensions.value("width").toDouble(200.0);
    loadedBox.height = dimensions.value("height").toDouble(200.0);
    loadedBox.unit = dimensions.value("unit").toString("mm");
    loadedBox.filePath = QFileInfo(filePath).absoluteFilePath();

    *box = loadedBox;
    return true;
}
