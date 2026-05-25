#pragma once

#include <functional>

class DiagnosticPanel;
class LogPanel;
class ProjectTreePanel;
class PropertyPanel;
class QMainWindow;
class RenderView;
class ResultPostprocessPanel;
class QString;
struct PickSelection;
struct Selection;

struct MainWindowDockWidgets
{
    ProjectTreePanel *projectTreePanel = nullptr;
    DiagnosticPanel *diagnosticPanel = nullptr;
    PropertyPanel *propertyPanel = nullptr;
    ResultPostprocessPanel *resultPostprocessPanel = nullptr;
    LogPanel *logPanel = nullptr;
    RenderView *renderView = nullptr;
};

struct MainWindowDockCallbacks
{
    std::function<void(const PickSelection &)> facePicked;
    std::function<void(const Selection &)> selectionChanged;
    std::function<void(const QString &)> resultFieldChanged;
    std::function<void(double)> resultDeformationScaleChanged;
    std::function<void(bool)> resultMeshEdgesChanged;
    std::function<void(bool)> resultUndeformedOverlayChanged;
    std::function<void(double)> resultAnimationPlayRequested;
    std::function<void()> resultAnimationStopRequested;
    std::function<void()> resultExportCsvRequested;
    std::function<void()> resultExportReportRequested;
    std::function<void()> resultExportScreenshotRequested;
    std::function<void()> resultOpenDirectoryRequested;
    std::function<void()> resultRenameRequested;
    std::function<void()> resultDeleteRequested;
};

class MainWindowDockBuilder
{
public:
    static MainWindowDockWidgets build(QMainWindow *window, const MainWindowDockCallbacks &callbacks);
};
