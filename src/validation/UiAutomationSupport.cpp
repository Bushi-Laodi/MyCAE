#include "validation/UiAutomationSupport.h"

#include "ui/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

namespace
{
constexpr const char *RecentProjectsKey = "projects/recent";
}

namespace UiAutomationSupport
{
RecentProjectsSettingsGuard::RecentProjectsSettingsGuard()
    : m_hadValue(m_settings.contains(RecentProjectsKey))
    , m_value(m_settings.value(RecentProjectsKey))
{
    m_settings.remove(RecentProjectsKey);
    m_settings.sync();
}

RecentProjectsSettingsGuard::~RecentProjectsSettingsGuard()
{
    if (m_hadValue) {
        m_settings.setValue(RecentProjectsKey, m_value);
    } else {
        m_settings.remove(RecentProjectsKey);
    }
    m_settings.sync();
}

void RecentProjectsSettingsGuard::setRecentProjects(const QStringList &projects)
{
    m_settings.setValue(RecentProjectsKey, projects);
    m_settings.sync();
}

QString demoProjectFilePath()
{
    const QString appProject = QDir(QCoreApplication::applicationDirPath())
        .filePath("samples/projects/box_pressure_demo/project.json");
    if (QFileInfo::exists(appProject)) {
        return appProject;
    }

#ifdef MYCAE_SOURCE_DIR
    const QString sourceProject = QDir(QString::fromUtf8(MYCAE_SOURCE_DIR))
        .filePath("samples/projects/box_pressure_demo/project.json");
    if (QFileInfo::exists(sourceProject)) {
        return sourceProject;
    }
#endif

    return appProject;
}

QTreeWidgetItem *findTreeItemByText(QTreeWidgetItem *root, const QString &text)
{
    if (!root) {
        return nullptr;
    }
    if (root->text(0) == text) {
        return root;
    }
    for (int i = 0; i < root->childCount(); ++i) {
        if (QTreeWidgetItem *match = findTreeItemByText(root->child(i), text)) {
            return match;
        }
    }
    return nullptr;
}

QTreeWidgetItem *findTreeItemByText(const QMainWindow &window, const QString &text)
{
    QTreeWidget *tree = window.findChild<QTreeWidget *>();
    if (!tree) {
        return nullptr;
    }
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        if (QTreeWidgetItem *match = findTreeItemByText(tree->topLevelItem(i), text)) {
            return match;
        }
    }
    return nullptr;
}

bool selectTreeItem(const QMainWindow &window, const QString &text)
{
    QTreeWidget *tree = window.findChild<QTreeWidget *>();
    QTreeWidgetItem *item = findTreeItemByText(window, text);
    if (!tree || !item) {
        return false;
    }
    tree->setCurrentItem(item);
    return true;
}

void enableStableResultSummaryMode(MainWindow &window)
{
    for (QWidget *widget : window.findChildren<QWidget *>()) {
        widget->setProperty("mycae.skipResultRender", true);
    }
}

void setSkipVtkCanvas(bool enabled)
{
    if (qApp) {
        qApp->setProperty("mycae.skipVtkCanvas", enabled);
    }
}
}
