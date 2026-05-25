#include "result/ResultManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
QString absoluteResultsPath(const Project &project)
{
    return QDir(project.rootPath).filePath(ResultManager::relativeResultsFilePath());
}

QJsonArray toJsonArray(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values) {
        array.append(value);
    }
    return array;
}

QStringList toStringList(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value : array) {
        values.append(value.toString());
    }
    return values;
}

QJsonObject toJsonObject(const ResultObject &result)
{
    QJsonObject object;
    object.insert("id", result.id);
    object.insert("name", result.name);
    object.insert("solverName", result.solverName);
    object.insert("meshName", result.meshName);
    object.insert("casePath", result.casePath);
    object.insert("logFile", result.logFile);
    object.insert("datFile", result.datFile);
    object.insert("frdFile", result.frdFile);
    object.insert("staFile", result.staFile);
    object.insert("resultFiles", toJsonArray(result.resultFiles));
    object.insert("availableFields", toJsonArray(result.availableFields));
    object.insert("primaryFieldName", result.primaryFieldName);
    object.insert("displayFieldName", result.displayFieldName);
    object.insert("deformationScale", result.deformationScale);
    object.insert("showMeshEdges", result.showMeshEdges);
    object.insert("showUndeformedOverlay", result.showUndeformedOverlay);
    object.insert("resultFilesComplete", result.resultFilesComplete);
    object.insert("matchedNodeCount", result.matchedNodeCount);
    object.insert("meshNodeCount", result.meshNodeCount);
    object.insert("matchedElementCount", result.matchedElementCount);
    object.insert("meshElementCount", result.meshElementCount);
    object.insert("scalarMin", result.scalarMin);
    object.insert("scalarMax", result.scalarMax);
    object.insert("checkMessages", toJsonArray(result.checkMessages));
    object.insert("createdAt", result.createdAt);
    object.insert("success", result.success);
    object.insert("summary", result.summary);
    return object;
}

ResultObject fromJsonObject(const QJsonObject &object)
{
    ResultObject result;
    result.id = object.value("id").toString();
    result.name = object.value("name").toString();
    result.solverName = object.value("solverName").toString();
    result.meshName = object.value("meshName").toString();
    result.casePath = object.value("casePath").toString();
    result.logFile = object.value("logFile").toString();
    result.datFile = object.value("datFile").toString();
    result.frdFile = object.value("frdFile").toString();
    result.staFile = object.value("staFile").toString();
    result.resultFiles = toStringList(object.value("resultFiles").toArray());
    result.availableFields = toStringList(object.value("availableFields").toArray());
    result.primaryFieldName = object.value("primaryFieldName").toString();
    result.displayFieldName = object.value("displayFieldName").toString(result.primaryFieldName);
    result.deformationScale = object.value("deformationScale").toDouble(0.0);
    result.showMeshEdges = object.value("showMeshEdges").toBool(true);
    result.showUndeformedOverlay = object.value("showUndeformedOverlay").toBool(false);
    result.resultFilesComplete = object.value("resultFilesComplete").toBool(false);
    result.matchedNodeCount = object.value("matchedNodeCount").toInt(0);
    result.meshNodeCount = object.value("meshNodeCount").toInt(0);
    result.matchedElementCount = object.value("matchedElementCount").toInt(0);
    result.meshElementCount = object.value("meshElementCount").toInt(0);
    result.scalarMin = object.value("scalarMin").toDouble(0.0);
    result.scalarMax = object.value("scalarMax").toDouble(0.0);
    result.checkMessages = toStringList(object.value("checkMessages").toArray());
    result.createdAt = object.value("createdAt").toString();
    result.success = object.value("success").toBool(false);
    result.summary = object.value("summary").toString();
    return result;
}
}

QString ResultManager::relativeResultsFilePath()
{
    return QDir("solver").filePath("results.json");
}

bool ResultManager::exists(const Project &project) const
{
    return QFileInfo::exists(absoluteResultsPath(project));
}

bool ResultManager::load(const Project &project, std::vector<ResultObject> &results, QString *errorMessage) const
{
    results.clear();

    QFile file(absoluteResultsPath(project));
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = "Open result index failed: " + file.errorString();
        }
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        if (errorMessage) {
            *errorMessage = "Result index is invalid: root is not an object.";
        }
        return false;
    }

    const QJsonArray array = document.object().value("results").toArray();
    results.reserve(array.size());
    for (const QJsonValue &value : array) {
        if (value.isObject()) {
            results.push_back(fromJsonObject(value.toObject()));
        }
    }
    return true;
}

bool ResultManager::save(const Project &project, const std::vector<ResultObject> &results, QString *errorMessage) const
{
    const QString filePath = absoluteResultsPath(project);
    if (!QDir().mkpath(QFileInfo(filePath).absolutePath())) {
        if (errorMessage) {
            *errorMessage = "Create result index directory failed: " + QFileInfo(filePath).absolutePath();
        }
        return false;
    }

    QJsonArray array;
    for (const ResultObject &result : results) {
        array.append(toJsonObject(result));
    }

    QJsonObject root;
    root.insert("version", "0.1.0");
    root.insert("results", array);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Write result index failed: " + file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}
