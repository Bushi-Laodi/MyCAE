#pragma once

#include <QString>
#include <QStringList>

#include <functional>

class AppSettings;
class ProjectModel;
class ResultAnimationController;
class ResultWorkflowController;
class QWidget;
struct ResultProbe;
struct MainWindowDockWidgets;

struct MainWindowResultCallbacks
{
    std::function<void(const QStringList &)> writeLogMessages;
    std::function<void()> updateActionStates;
};

struct MainWindowResultContext
{
    ProjectModel &projectModel;
    MainWindowDockWidgets &docks;
    AppSettings &appSettings;
    ResultAnimationController &resultAnimationController;
    QWidget *parent = nullptr;
};

class MainWindowResultController
{
public:
    MainWindowResultController(MainWindowResultContext context, MainWindowResultCallbacks callbacks);

    void setSelectedField(const QString &fieldName) const;
    void setSelectedDeformationScale(double scale) const;
    void setSelectedMeshEdges(bool enabled) const;
    void setSelectedUndeformedOverlay(bool enabled) const;
    void setSelectedScalarRangeLock(bool locked) const;
    void setSelectedScalarRange(double minimum, double maximum) const;
    void setSelectedProbe(const ResultProbe &probe) const;
    void playSelectedAnimation(double speed) const;
    void stopSelectedAnimation() const;
    void applyAnimatedDeformationScale(double scale) const;
    void exportSelectedCsv() const;
    void exportSelectedReport() const;
    void exportRenderScreenshot() const;
    void openSelectedResultDirectory() const;
    void renameSelectedResult() const;
    void deleteSelectedResultHistory() const;

private:
    ResultWorkflowController workflow() const;
    void writeMessages(const QStringList &messages) const;
    void writeMessagesAndUpdate(const QStringList &messages) const;

    MainWindowResultContext m_context;
    MainWindowResultCallbacks m_callbacks;
};
