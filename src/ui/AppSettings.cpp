#include "ui/AppSettings.h"

#include <QFileInfo>
#include <QMainWindow>
#include <QSettings>

#include <algorithm>

namespace
{
constexpr int MaxRecentProjects = 8;

QSettings settings()
{
    return QSettings("MyCAE", "MyCAE");
}
}

void AppSettings::restoreMainWindow(QMainWindow &window) const
{
    QSettings appSettings = settings();
    const QByteArray geometry = appSettings.value("mainWindow/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        window.restoreGeometry(geometry);
    }
    const QByteArray state = appSettings.value("mainWindow/state").toByteArray();
    if (!state.isEmpty()) {
        window.restoreState(state);
    }
}

void AppSettings::saveMainWindow(const QMainWindow &window) const
{
    QSettings appSettings = settings();
    appSettings.setValue("mainWindow/geometry", window.saveGeometry());
    appSettings.setValue("mainWindow/state", window.saveState());
}

QStringList AppSettings::recentProjects() const
{
    QStringList projects = settings().value("projects/recent").toStringList();
    projects.erase(std::remove_if(projects.begin(), projects.end(), [](const QString &path) {
        return !QFileInfo::exists(path);
    }), projects.end());
    return projects;
}

void AppSettings::addRecentProject(const QString &projectFilePath) const
{
    if (projectFilePath.trimmed().isEmpty()) {
        return;
    }

    QStringList projects = recentProjects();
    projects.removeAll(projectFilePath);
    projects.prepend(projectFilePath);
    while (projects.size() > MaxRecentProjects) {
        projects.removeLast();
    }

    QSettings appSettings = settings();
    appSettings.setValue("projects/recent", projects);
}

void AppSettings::clearRecentProjects() const
{
    settings().remove("projects/recent");
}

QString AppSettings::recentExportDirectory() const
{
    return settings().value("export/recentDirectory").toString();
}

void AppSettings::setRecentExportDirectory(const QString &directoryPath) const
{
    if (!directoryPath.trimmed().isEmpty()) {
        QSettings appSettings = settings();
        appSettings.setValue("export/recentDirectory", directoryPath);
    }
}
