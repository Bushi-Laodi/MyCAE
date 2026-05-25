#pragma once

#include <QString>
#include <QStringList>

class QMainWindow;

class AppSettings
{
public:
    void restoreMainWindow(QMainWindow &window) const;
    void saveMainWindow(const QMainWindow &window) const;

    QStringList recentProjects() const;
    void addRecentProject(const QString &projectFilePath) const;
    void clearRecentProjects() const;

    QString recentExportDirectory() const;
    void setRecentExportDirectory(const QString &directoryPath) const;
};
