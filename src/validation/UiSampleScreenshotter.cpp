#include "validation/UiSampleScreenshotter.h"

#include "ui/MainWindow.h"
#include "validation/UiAutomationSupport.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QPixmap>
#include <QStandardPaths>

#include <utility>

namespace
{
QString defaultOutputDirectory()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(base).filePath("MyCAE/ui_screenshots/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
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
}

UiSampleScreenshotter::UiSampleScreenshotter(QString outputDirectory)
    : m_outputDirectory(std::move(outputDirectory))
{
}

UiSampleScreenshotResult UiSampleScreenshotter::capture()
{
    UiSampleScreenshotResult result;
    result.outputDirectory = m_outputDirectory.isEmpty() ? defaultOutputDirectory() : m_outputDirectory;

    const QString projectFilePath = UiAutomationSupport::demoProjectFilePath();
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
        UiAutomationSupport::RecentProjectsSettingsGuard settingsGuard;
        MainWindow window;
        UiAutomationSupport::enableStableResultSummaryMode(window);
        window.resize(1360, 860);
        window.show();
        QApplication::processEvents();

        if (!window.openProjectFileForAutomation(projectFilePath)) {
            result.messages.append("Failed to open demo project: " + projectFilePath);
            return result;
        }
        QApplication::processEvents();

        const QString propertyScreenshot = outputDir.filePath("box_pressure_demo_properties.png");
        if (!UiAutomationSupport::selectTreeItem(window, "Box_1")) {
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
        if (!UiAutomationSupport::selectTreeItem(window, "CalculiX Result - Box Pressure Demo")) {
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
