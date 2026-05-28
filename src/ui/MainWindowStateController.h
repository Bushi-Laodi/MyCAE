#pragma once

class DiagnosticCollector;
class PickController;
class ProjectModel;
class RenderView;
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
        const RenderView *renderView,
        ResultPostprocessPanel *resultPostprocessPanel
    );
};
