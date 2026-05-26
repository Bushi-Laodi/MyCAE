#pragma once

#include <QString>
#include <QStringList>

class AppSettings;
class ProjectModel;
class ProjectTreePanel;
class PropertyPanel;
class RenderView;
class ResultAnimationController;
class ResultPostprocessPanel;
class QWidget;
struct ResultProbe;

class ResultWorkflowController
{
public:
    ResultWorkflowController(
        ProjectModel &projectModel,
        ProjectTreePanel *projectTreePanel,
        PropertyPanel *propertyPanel,
        ResultPostprocessPanel *resultPostprocessPanel,
        RenderView *renderView,
        AppSettings &appSettings,
        ResultAnimationController &animationController,
        QWidget *parentWidget
    );

    QStringList setSelectedField(const QString &fieldName);
    QStringList setSelectedDeformationScale(double scale);
    QStringList setSelectedMeshEdges(bool enabled);
    QStringList setSelectedUndeformedOverlay(bool enabled);
    QStringList setSelectedScalarRangeLock(bool locked);
    QStringList setSelectedScalarRange(double minimum, double maximum);
    QStringList setSelectedProbe(const ResultProbe &probe);
    QStringList playSelectedAnimation(double speed);
    QStringList stopSelectedAnimation();
    QStringList applyAnimatedDeformationScale(double scale);
    QStringList exportSelectedCsv();
    QStringList exportSelectedReport();
    QStringList exportRenderScreenshot();
    QStringList openSelectedResultDirectory();
    QStringList renameSelectedResult();
    QStringList deleteSelectedResultHistory();

private:
    QStringList redisplaySelectedResult(bool resetCamera = true);
    void saveResultIndex(QStringList &logMessages);
    void refreshResultPanels();

    ProjectModel &m_projectModel;
    ProjectTreePanel *m_projectTreePanel = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    ResultPostprocessPanel *m_resultPostprocessPanel = nullptr;
    RenderView *m_renderView = nullptr;
    AppSettings &m_appSettings;
    ResultAnimationController &m_animationController;
    QWidget *m_parentWidget = nullptr;
};
