#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

class ProjectModel;
struct ResultObject;

enum class ProjectResourceStatus
{
    Valid,
    Missing,
    Incomplete
};

struct ProjectResourceItem
{
    QString category;
    QString name;
    QString path;
    qint64 sizeBytes = 0;
    ProjectResourceStatus status = ProjectResourceStatus::Valid;
    QString detail;
    QString resultId;
};

struct ProjectResourceReport
{
    QVector<ProjectResourceItem> items;
    qint64 totalBytes = 0;
    QStringList invalidResultIds;
};

QString projectResourceStatusName(ProjectResourceStatus status);
QString projectResourceSizeText(qint64 sizeBytes);

class ProjectResourceManager
{
public:
    ProjectResourceReport inspect(const ProjectModel &projectModel) const;

    bool removeInvalidResultRecords(ProjectModel &projectModel, QStringList *removedIds, QString *errorMessage) const;
    bool deleteResultFiles(ProjectModel &projectModel, const QString &resultId, QString *errorMessage) const;
};
