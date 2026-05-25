#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class DiagnosticCollector;
class ProjectModel;
struct MainWindowDockWidgets;

struct MainWindowViewCallbacks
{
    std::function<void()> updateActionStates;
};

struct MainWindowViewContext
{
    ProjectModel &projectModel;
    MainWindowDockWidgets &docks;
    DiagnosticCollector &diagnosticCollector;
};

class MainWindowViewController
{
public:
    MainWindowViewController(MainWindowViewContext context, MainWindowViewCallbacks callbacks);

    void refreshDiagnosticsPanel() const;
    void refreshResultViews() const;
    void writeLog(const QString &message) const;
    void writeLogMessages(const QStringList &messages) const;

private:
    MainWindowViewContext m_context;
    MainWindowViewCallbacks m_callbacks;
};
