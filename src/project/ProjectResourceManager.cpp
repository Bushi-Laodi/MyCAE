#include "project/ProjectResourceManager.h"

#include "project/ProjectModel.h"
#include "result/ResultManager.h"
#include "result/ResultObject.h"

#include <QDir>
#include <QFileInfo>

#include <algorithm>

namespace
{
QString absoluteProjectPath(const ProjectModel &projectModel, const QString &relativePath)
{
    const QFileInfo fileInfo(relativePath);
    if (fileInfo.isAbsolute()) {
        return fileInfo.absoluteFilePath();
    }
    return QDir(projectModel.project().rootPath).filePath(relativePath);
}

qint64 directorySize(const QString &path)
{
    qint64 size = 0;
    const QDir directory(path);
    const QFileInfoList entries =
        directory.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        size += entry.isDir() ? directorySize(entry.absoluteFilePath()) : entry.size();
    }
    return size;
}

qint64 pathSize(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists()) {
        return 0;
    }
    return info.isDir() ? directorySize(info.absoluteFilePath()) : info.size();
}

void addFileItem(
    ProjectResourceReport &report,
    const QString &category,
    const QString &name,
    const QString &path,
    const QString &detail = {}
)
{
    ProjectResourceItem item;
    item.category = category;
    item.name = name;
    item.path = path;
    item.sizeBytes = pathSize(path);
    item.status = QFileInfo::exists(path) ? ProjectResourceStatus::Valid : ProjectResourceStatus::Missing;
    item.detail = detail;
    report.items.append(item);
}

bool resultHasRequiredFiles(const ResultObject &resultObject)
{
    return QFileInfo::exists(resultObject.casePath)
        && QFileInfo::exists(resultObject.datFile)
        && QFileInfo::exists(resultObject.staFile)
        && QFileInfo::exists(resultObject.frdFile);
}

QString resultDetail(const ResultObject &resultObject)
{
    QStringList missing;
    if (!QFileInfo::exists(resultObject.casePath)) {
        missing.append("case directory");
    }
    if (!QFileInfo::exists(resultObject.datFile)) {
        missing.append(".dat");
    }
    if (!QFileInfo::exists(resultObject.staFile)) {
        missing.append(".sta");
    }
    if (!QFileInfo::exists(resultObject.frdFile)) {
        missing.append(".frd");
    }
    return missing.isEmpty() ? resultObject.summary : "Missing " + missing.join(", ");
}
}

QString projectResourceStatusName(ProjectResourceStatus status)
{
    switch (status) {
    case ProjectResourceStatus::Valid:
        return "Valid";
    case ProjectResourceStatus::Missing:
        return "Missing";
    case ProjectResourceStatus::Incomplete:
        return "Incomplete";
    }
    return "Unknown";
}

QString projectResourceSizeText(qint64 sizeBytes)
{
    constexpr double KiB = 1024.0;
    constexpr double MiB = KiB * 1024.0;
    constexpr double GiB = MiB * 1024.0;
    if (sizeBytes >= GiB) {
        return QString::number(sizeBytes / GiB, 'f', 2) + " GB";
    }
    if (sizeBytes >= MiB) {
        return QString::number(sizeBytes / MiB, 'f', 2) + " MB";
    }
    if (sizeBytes >= KiB) {
        return QString::number(sizeBytes / KiB, 'f', 2) + " KB";
    }
    return QString::number(sizeBytes) + " B";
}

ProjectResourceReport ProjectResourceManager::inspect(const ProjectModel &projectModel) const
{
    ProjectResourceReport report;
    if (!projectModel.hasProject()) {
        return report;
    }

    const QString rootPath = projectModel.project().rootPath;
    report.totalBytes = pathSize(rootPath);
    addFileItem(report, "Project", "project.json", projectModel.project().projectFilePath);
    addFileItem(report, "Project", "geometry", QDir(rootPath).filePath("geometry"));
    addFileItem(report, "Project", "mesh", QDir(rootPath).filePath("mesh"));
    addFileItem(report, "Project", "solver", QDir(rootPath).filePath("solver"));
    addFileItem(report, "Result", "results.json", QDir(rootPath).filePath(ResultManager::relativeResultsFilePath()));

    for (const auto &geometry : projectModel.geometryObjects()) {
        addFileItem(report, "Geometry", geometry.name, absoluteProjectPath(projectModel, geometry.brepFile), geometry.type);
    }
    for (const auto &mesh : projectModel.meshObjects()) {
        addFileItem(report, "Mesh", mesh.name, absoluteProjectPath(projectModel, mesh.mshFile), "MSH mesh");
    }
    for (const ResultObject &result : projectModel.resultRepository().results()) {
        ProjectResourceItem item;
        item.category = "Result";
        item.name = result.name;
        item.path = result.casePath;
        item.sizeBytes = pathSize(result.casePath);
        item.status = resultHasRequiredFiles(result) ? ProjectResourceStatus::Valid : ProjectResourceStatus::Incomplete;
        item.detail = resultDetail(result);
        item.resultId = result.id;
        report.items.append(item);
        if (item.status != ProjectResourceStatus::Valid) {
            report.invalidResultIds.append(result.id);
        }
    }

    return report;
}

bool ProjectResourceManager::removeInvalidResultRecords(
    ProjectModel &projectModel,
    QStringList *removedIds,
    QString *errorMessage
) const
{
    if (removedIds) {
        removedIds->clear();
    }
    std::vector<ResultObject> &results = projectModel.resultRepository().results();
    const auto oldEnd = std::remove_if(results.begin(), results.end(), [removedIds](const ResultObject &result) {
        const bool invalid = !resultHasRequiredFiles(result);
        if (invalid && removedIds) {
            removedIds->append(result.id);
        }
        return invalid;
    });
    results.erase(oldEnd, results.end());

    if (!projectModel.hasProject()) {
        return true;
    }
    return ResultManager().save(projectModel.project(), results, errorMessage);
}

bool ProjectResourceManager::deleteResultFiles(ProjectModel &projectModel, const QString &resultId, QString *errorMessage) const
{
    ResultObject *result = projectModel.findResultById(resultId);
    if (!result) {
        if (errorMessage) {
            *errorMessage = "Result record not found: " + resultId;
        }
        return false;
    }
    if (!result->casePath.isEmpty() && QFileInfo::exists(result->casePath)) {
        QDir directory(result->casePath);
        if (!directory.removeRecursively()) {
            if (errorMessage) {
                *errorMessage = "Delete result directory failed: " + result->casePath;
            }
            return false;
        }
    }
    return true;
}
