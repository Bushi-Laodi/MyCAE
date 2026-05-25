#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class DiagnosticCollector;
class ProjectModel;
class QWidget;
struct MainWindowDockWidgets;

struct MainWindowToolCallbacks
{
    std::function<void(const QString &)> writeLog;
    std::function<void(const QStringList &)> writeLogMessages;
    std::function<void()> refreshDiagnosticsPanel;
    std::function<void()> refreshResultViews;
};

struct MainWindowToolContext
{
    ProjectModel &projectModel;
    MainWindowDockWidgets &docks;
    DiagnosticCollector &diagnosticCollector;
    QWidget *parent = nullptr;
};

class MainWindowToolController
{
public:
    MainWindowToolController(MainWindowToolContext context, MainWindowToolCallbacks callbacks);

    void showProjectResources() const;
    void validateSamples() const;
    void clearDiagnostics() const;

private:
    MainWindowToolContext m_context;
    MainWindowToolCallbacks m_callbacks;
};
