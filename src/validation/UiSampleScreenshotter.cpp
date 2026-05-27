#include "validation/UiSampleScreenshotter.h"

#include "ui/MainWindow.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QPixmap>
#include <QSettings>
#include <QStandardPaths>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QWidget>

#include <utility>

namespace
{
constexpr const char *RecentProjectsKey = "projects/recent";

QString defaultDemoProjectFilePath()
{
    const QString appProject = QDir(QApplication::applicationDirPath())
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

QString defaultOutputDirectory()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(base).filePath("MyCAE/ui_screenshots/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
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

bool raiseDock(QMainWindow &window, const QString &title)
{
    const QList<QDockWidget *> docks = window.findChildren<QDockWidget *>();
    for (QDockWidget *dock : docks) {
        if (dock && dock->windowTitle() == title) {
            dock->show();
            dock->raise();
            return true;
        }
    }
    return false;
}

bool saveWindowScreenshot(QMainWindow &window, const QString &filePath)
{
    QApplication::processEvents();
    const QPixmap pixmap = window.grab();
    return !pixmap.isNull() && pixmap.save(filePath);
}

class RecentProjectsSettingsGuard
{
public:
    RecentProjectsSettingsGuard()
        : m_hadValue(m_settings.contains(RecentProjectsKey))
        , m_value(m_settings.value(RecentProjectsKey))
    {
    }

    ~RecentProjectsSettingsGuard()
    {
        if (m_hadValue) {
            m_settings.setValue(RecentProjectsKey, m_value);
        } else {
            m_settings.remove(RecentProjectsKey);
        }
        m_settings.sync();
    }

private:
    QSettings m_settings{"MyCAE", "MyCAE"};
    bool m_hadValue = false;
    QVariant m_value;
};
}

UiSampleScreenshotter::UiSampleScreenshotter(QString outputDirectory)
    : m_outputDirectory(std::move(outputDirectory))
{
}

UiSampleScreenshotResult UiSampleScreenshotter::capture()
{
    UiSampleScreenshotResult result;
    result.outputDirectory = m_outputDirectory.isEmpty() ? defaultOutputDirectory() : m_outputDirectory;

    const QString projectFilePath = defaultDemoProjectFilePath();
    if (!QFileInfo::exists(projectFilePath)) {
        result.messages.append("Demo project file not found: " + projectFilePath);
        return result;
    }

    QDir outputDir(result.outputDirectory);
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        result.messages.append("Failed to create screenshot directory: " + result.outputDirectory);
        return result;
    }

    {
        RecentProjectsSettingsGuard settingsGuard;
        MainWindow window;
        for (QWidget *widget : window.findChildren<QWidget *>()) {
            widget->setProperty("mycae.skipResultRender", true);
        }
        window.resize(1360, 860);
        window.show();
        QApplication::processEvents();

        if (!window.openProjectFileForAutomation(projectFilePath)) {
            result.messages.append("Failed to open demo project: " + projectFilePath);
            return result;
        }
        QApplication::processEvents();

        const QString propertyScreenshot = outputDir.filePath("box_pressure_demo_properties.png");
        if (!selectTreeItem(window, "Box_1")) {
            result.messages.append("Failed to select demo geometry Box_1.");
            return result;
        }
        raiseDock(window, QString::fromUtf8(u8"属性"));
        QApplication::processEvents();
        if (!saveWindowScreenshot(window, propertyScreenshot)) {
            result.messages.append("Failed to save screenshot: " + propertyScreenshot);
            return result;
        }
        result.screenshotFiles.append(propertyScreenshot);

        const QString resultScreenshot = outputDir.filePath("box_pressure_demo_result_postprocess.png");
        if (!selectTreeItem(window, "CalculiX Result - Box Pressure Demo")) {
            result.messages.append("Failed to select demo result.");
            return result;
        }
        raiseDock(window, QString::fromUtf8(u8"结果后处理"));
        QApplication::processEvents();
        if (!saveWindowScreenshot(window, resultScreenshot)) {
            result.messages.append("Failed to save screenshot: " + resultScreenshot);
            return result;
        }
        result.screenshotFiles.append(resultScreenshot);
    }

    result.messages.append("Captured UI screenshots for box_pressure_demo.");
    result.success = true;
    return result;
}
