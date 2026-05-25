#pragma once

#include <QProcessEnvironment>
#include <QString>

class CalculiXEnvironment
{
public:
    static QString executablePath();
    static bool isExplicitExecutablePath(const QString &executablePath);
    static QProcessEnvironment processEnvironment(const QString &executablePath);
};
