#pragma once

#include <QString>

struct ResultObject
{
    QString id;
    QString name;
    QString solverName;
    QString casePath;
    QString logFile;
    QString createdAt;
    bool success = false;
    QString summary;
};
