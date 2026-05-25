#include "validation/UiSmokeValidator.h"

#include "ui/MainWindow.h"

#include <QAction>
#include <QDockWidget>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QStringList>
#include <QToolBar>

namespace
{
QString normalizedText(QString text)
{
    return text.remove('&');
}

QAction *findActionByCommandId(const QMainWindow &window, const QString &commandId)
{
    const QList<QAction *> actions = window.findChildren<QAction *>();
    for (QAction *action : actions) {
        if (action && action->data().toString() == commandId) {
            return action;
        }
    }
    return nullptr;
}

QAction *findActionByText(const QMainWindow &window, const QString &text)
{
    const QList<QAction *> actions = window.findChildren<QAction *>();
    for (QAction *action : actions) {
        if (action && normalizedText(action->text()) == text) {
            return action;
        }
    }
    return nullptr;
}

QMenu *findTopLevelMenu(const QMainWindow &window, const QString &title)
{
    if (!window.menuBar()) {
        return nullptr;
    }

    for (QAction *action : window.menuBar()->actions()) {
        if (action && action->menu() && normalizedText(action->text()) == title) {
            return action->menu();
        }
    }
    return nullptr;
}

QMenu *findSubMenu(const QMenu *menu, const QString &title)
{
    if (!menu) {
        return nullptr;
    }

    for (QAction *action : menu->actions()) {
        if (action && action->menu() && normalizedText(action->text()) == title) {
            return action->menu();
        }
    }
    return nullptr;
}

QAction *findMenuActionByText(const QMenu *menu, const QString &text)
{
    if (!menu) {
        return nullptr;
    }

    for (QAction *action : menu->actions()) {
        if (action && normalizedText(action->text()) == text) {
            return action;
        }
    }
    return nullptr;
}

int countActionsBeforeFirstSeparator(const QMenu *menu)
{
    if (!menu) {
        return 0;
    }

    int count = 0;
    for (QAction *action : menu->actions()) {
        if (!action) {
            continue;
        }
        if (action->isSeparator()) {
            break;
        }
        ++count;
    }
    return count;
}

bool hasDockWidgetTitle(const QMainWindow &window, const QString &title)
{
    const QList<QDockWidget *> docks = window.findChildren<QDockWidget *>();
    for (const QDockWidget *dock : docks) {
        if (dock && dock->windowTitle() == title) {
            return true;
        }
    }
    return false;
}

bool hasToolBarTitle(const QMainWindow &window, const QString &title)
{
    const QList<QToolBar *> toolBars = window.findChildren<QToolBar *>();
    for (const QToolBar *toolBar : toolBars) {
        if (toolBar && toolBar->windowTitle() == title && !toolBar->actions().isEmpty()) {
            return true;
        }
    }
    return false;
}

void addStep(UiValidationReport &report, const QString &name, bool passed, const QString &detail = {})
{
    report.steps.append(UiValidationStep{name, passed, detail});
}

void addActionExistsStep(UiValidationReport &report, const QMainWindow &window, const QString &commandId)
{
    addStep(
        report,
        "Action registered: " + commandId,
        findActionByCommandId(window, commandId) != nullptr
    );
}

void addActionDisabledStep(UiValidationReport &report, const QMainWindow &window, const QString &commandId)
{
    QAction *action = findActionByCommandId(window, commandId);
    addStep(
        report,
        "Action disabled without project: " + commandId,
        action && !action->isEnabled(),
        action ? QString() : "Action not found"
    );
}

class RecentProjectsSettingsGuard
{
public:
    RecentProjectsSettingsGuard()
        : m_hadValue(m_settings.contains(RecentProjectsKey))
        , m_value(m_settings.value(RecentProjectsKey))
    {
        m_settings.remove(RecentProjectsKey);
    }

    ~RecentProjectsSettingsGuard()
    {
        if (m_hadValue) {
            m_settings.setValue(RecentProjectsKey, m_value);
        } else {
            m_settings.remove(RecentProjectsKey);
        }
    }

private:
    static constexpr const char *RecentProjectsKey = "projects/recent";

    QSettings m_settings{"MyCAE", "MyCAE"};
    bool m_hadValue = false;
    QVariant m_value;
};
}

bool UiValidationReport::success() const
{
    for (const UiValidationStep &step : steps) {
        if (!step.passed) {
            return false;
        }
    }
    return true;
}

int UiValidationReport::passedCount() const
{
    int count = 0;
    for (const UiValidationStep &step : steps) {
        if (step.passed) {
            ++count;
        }
    }
    return count;
}

int UiValidationReport::failedCount() const
{
    return steps.size() - passedCount();
}

UiValidationReport UiSmokeValidator::validate() const
{
    UiValidationReport report;
    RecentProjectsSettingsGuard recentProjectsSettingsGuard;
    MainWindow window;

    const QStringList menuTitles{
        "File",
        "Edit",
        "Geometry",
        "Picking",
        "Solver Setup",
        "Mesh",
        "Simulation",
        "Postprocess",
        "Tools"
    };
    for (const QString &title : menuTitles) {
        addStep(report, "Menu exists: " + title, findTopLevelMenu(window, title) != nullptr);
    }
    QMenu *fileMenu = findTopLevelMenu(window, "File");
    QMenu *recentProjectsMenu = findSubMenu(fileMenu, "Recent Projects");
    addStep(report, "Recent Projects submenu exists", recentProjectsMenu != nullptr);
    addStep(
        report,
        "Recent Projects submenu has 8 slots",
        countActionsBeforeFirstSeparator(recentProjectsMenu) == 8,
        recentProjectsMenu ? QString("slots=%1").arg(countActionsBeforeFirstSeparator(recentProjectsMenu)) : "Menu not found"
    );
    QAction *clearRecentProjectsAction = findMenuActionByText(recentProjectsMenu, "Clear Recent Projects");
    addStep(
        report,
        "Clear Recent Projects is disabled initially",
        clearRecentProjectsAction && !clearRecentProjectsAction->isEnabled(),
        clearRecentProjectsAction ? QString() : "Action not found"
    );

    const QStringList dockTitles{
        "Project / Model",
        "Diagnostics",
        "Properties",
        "Result Postprocess",
        "Log"
    };
    for (const QString &title : dockTitles) {
        addStep(report, "Dock exists: " + title, hasDockWidgetTitle(window, title));
    }
    addStep(report, "Toolbar exists: Main Toolbar", hasToolBarTitle(window, "Main Toolbar"));

    const QStringList requiredCommandIds{
        "project.new",
        "project.open",
        "app.exit",
        "geometry.create.box",
        "geometry.create.cylinder",
        "geometry.import.step",
        "mesh.checkGmsh",
        "mesh.generate",
        "mesh.readInfo",
        "mesh.show",
        "picking.face",
        "picking.clear",
        "picking.faceGroup.createFromPick",
        "picking.faceGroup.addPicked",
        "picking.faceGroup.removePicked",
        "picking.faceGroup.clearFaces",
        "picking.faceGroup.rename",
        "picking.faceGroup.delete",
        "picking.faceGroup.localMeshSize",
        "picking.faceGroup.togglePhysicalGroup",
        "solverData.create.material",
        "solverData.create.boundaryCondition",
        "solverData.create.load",
        "solverData.editSelected",
        "solverData.deleteSelected",
        "simulation.generateMesh",
        "solver.run.calculix"
    };
    for (const QString &commandId : requiredCommandIds) {
        addActionExistsStep(report, window, commandId);
    }

    const QStringList disabledWithoutProject{
        "geometry.create.box",
        "geometry.create.cylinder",
        "mesh.generate",
        "mesh.readInfo",
        "mesh.show",
        "picking.face",
        "picking.faceGroup.createFromPick",
        "solverData.create.material",
        "solverData.create.boundaryCondition",
        "solverData.create.load",
        "solverData.editSelected",
        "solverData.deleteSelected",
        "solver.run.calculix"
    };
    for (const QString &commandId : disabledWithoutProject) {
        addActionDisabledStep(report, window, commandId);
    }

    QAction *projectResourcesAction = findActionByText(window, "Project Resources");
    addStep(
        report,
        "Action disabled without project: Project Resources",
        projectResourcesAction && !projectResourcesAction->isEnabled(),
        projectResourcesAction ? QString() : "Action not found"
    );
    QAction *resultFieldAction = findActionByText(window, "Ux");
    addStep(
        report,
        "Result field actions are disabled without project",
        resultFieldAction && !resultFieldAction->isEnabled(),
        resultFieldAction ? QString() : "Action not found"
    );

    return report;
}

QString uiValidationStatusName(bool passed)
{
    return passed ? "PASS" : "FAIL";
}
