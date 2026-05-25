#pragma once

class DiagnosticCollector;
class PickController;
class ProjectModel;
class ResultPostprocessPanel;
struct MainWindowActions;

class MainWindowStateController
{
public:
    static void update(
        const MainWindowActions &actions,
        const ProjectModel &projectModel,
        const PickController &pickController,
        const DiagnosticCollector &diagnosticCollector,
        ResultPostprocessPanel *resultPostprocessPanel
    );
};
