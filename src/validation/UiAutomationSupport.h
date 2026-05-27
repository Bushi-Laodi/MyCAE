#pragma once

#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

class MainWindow;
class QMainWindow;
class QTreeWidgetItem;

namespace UiAutomationSupport
{
class RecentProjectsSettingsGuard
{
public:
    RecentProjectsSettingsGuard();
    ~RecentProjectsSettingsGuard();

    void setRecentProjects(const QStringList &projects);

private:
    QSettings m_settings{"MyCAE", "MyCAE"};
    bool m_hadValue = false;
    QVariant m_value;
};

QString demoProjectFilePath();
QTreeWidgetItem *findTreeItemByText(QTreeWidgetItem *root, const QString &text);
QTreeWidgetItem *findTreeItemByText(const QMainWindow &window, const QString &text);
bool selectTreeItem(const QMainWindow &window, const QString &text);
void enableStableResultSummaryMode(MainWindow &window);
void setSkipVtkCanvas(bool enabled);
}
