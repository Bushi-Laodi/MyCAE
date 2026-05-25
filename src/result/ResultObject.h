#pragma once

#include <QString>
#include <QStringList>

struct ResultObject
{
    QString id;
    QString name;
    QString solverName;
    QString meshName;
    QString casePath;
    QString logFile;
    QString datFile;
    QString frdFile;
    QString staFile;
    QStringList resultFiles;
    QStringList availableFields;
    QString primaryFieldName;
    QString createdAt;
    bool success = false;
    QString summary;
};
