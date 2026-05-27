#include "validation/UiSmokeValidator.h"

#include "ui/MainWindow.h"
#include "ui/PropertyPanel.h"
#include "ui/ResultPostprocessPanel.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDockWidget>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStringList>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

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

int topLevelMenuCount(const QMainWindow &window)
{
    return window.menuBar() ? window.menuBar()->actions().size() : 0;
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

bool hasIconOnlyToolBar(const QMainWindow &window, const QString &title)
{
    const QList<QToolBar *> toolBars = window.findChildren<QToolBar *>();
    for (const QToolBar *toolBar : toolBars) {
        if (toolBar && toolBar->windowTitle() == title) {
            return toolBar->toolButtonStyle() == Qt::ToolButtonIconOnly;
        }
    }
    return false;
}

bool applicationStyleContains(const QString &token)
{
    return qApp && qApp->styleSheet().contains(token);
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

void addActionEnabledStep(UiValidationReport &report, const QMainWindow &window, const QString &commandId)
{
    QAction *action = findActionByCommandId(window, commandId);
    addStep(
        report,
        "Action enabled after demo open: " + commandId,
        action && action->isEnabled(),
        action ? QString() : "Action not found"
    );
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

int treeItemChildCount(const QMainWindow &window, const QString &text)
{
    if (QTreeWidgetItem *item = findTreeItemByText(window, text)) {
        return item->childCount();
    }
    return -1;
}

bool treeItemHasCategoryStyle(const QMainWindow &window, const QString &text)
{
    QTreeWidgetItem *item = findTreeItemByText(window, text);
    return item && item->font(0).bold() && !item->icon(0).isNull();
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

bool resultFieldControlEnabled(const QMainWindow &window)
{
    QComboBox *fieldComboBox = window.findChild<QComboBox *>("result.field.combo");
    return fieldComboBox && fieldComboBox->isEnabled();
}

bool hasNamedWidget(const QMainWindow &window, const QString &objectName)
{
    return window.findChild<QWidget *>(objectName) != nullptr;
}

bool hasResizableScrollArea(const QMainWindow &window, const QString &objectName)
{
    const QScrollArea *scrollArea = window.findChild<QScrollArea *>(objectName);
    return scrollArea && scrollArea->widgetResizable() && scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
}

QString namedLabelText(const QMainWindow &window, const QString &objectName)
{
    const QLabel *label = window.findChild<QLabel *>(objectName);
    return label ? label->text() : QString();
}

bool namedButtonEnabled(const QMainWindow &window, const QString &objectName)
{
    const QPushButton *button = window.findChild<QPushButton *>(objectName);
    return button && button->isEnabled();
}

bool namedComboBoxEnabled(const QMainWindow &window, const QString &objectName)
{
    const QComboBox *comboBox = window.findChild<QComboBox *>(objectName);
    return comboBox && comboBox->isEnabled();
}

bool diagnosticEmptyStateVisible(const QMainWindow &window)
{
    const QTableWidget *table = window.findChild<QTableWidget *>("diagnostic.table");
    return table
        && table->rowCount() == 1
        && table->item(0, 2)
        && table->item(0, 2)->text() == zh(u8"暂无诊断信息。");
}

bool logPlaceholderConfigured(const QMainWindow &window)
{
    const QPlainTextEdit *logView = window.findChild<QPlainTextEdit *>("log.view");
    return logView && logView->placeholderText() == zh(u8"暂无日志消息。");
}

QString defaultDemoProjectFilePath()
{
    const QString appProject = QDir(QCoreApplication::applicationDirPath())
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

class RecentProjectsSettingsGuard
{
public:
    RecentProjectsSettingsGuard()
        : m_hadValue(m_settings.contains(RecentProjectsKey))
        , m_value(m_settings.value(RecentProjectsKey))
    {
        m_settings.remove(RecentProjectsKey);
        m_settings.sync();
    }

    void setRecentProjects(const QStringList &projects)
    {
        m_settings.setValue(RecentProjectsKey, projects);
        m_settings.sync();
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
    static constexpr const char *RecentProjectsKey = "projects/recent";

    QSettings m_settings{"MyCAE", "MyCAE"};
    bool m_hadValue = false;
    QVariant m_value;
};

void validateDemoProjectUi(UiValidationReport &report, RecentProjectsSettingsGuard &settingsGuard, MainWindow &window)
{
    const QString demoProjectFilePath = defaultDemoProjectFilePath();
    addStep(
        report,
        "Demo project file exists",
        QFileInfo::exists(demoProjectFilePath),
        demoProjectFilePath
    );
    if (!QFileInfo::exists(demoProjectFilePath)) {
        return;
    }

    settingsGuard.setRecentProjects(QStringList{demoProjectFilePath});
    QMenu *recentProjectsMenu = findSubMenu(findTopLevelMenu(window, zh(u8"文件")), zh(u8"最近工程"));
    QAction *openDemoAction = nullptr;
    if (recentProjectsMenu) {
        for (QAction *action : recentProjectsMenu->actions()) {
            if (action && !action->isSeparator() && action->isVisible()) {
                openDemoAction = action;
                break;
            }
        }
    }
    addStep(report, "Demo project appears in Recent Projects", openDemoAction != nullptr);
    if (!openDemoAction) {
        window.prepareForAutomationShutdown();
        return;
    }

    window.openProjectFileForAutomation(demoProjectFilePath);

    addStep(
        report,
        "Demo project opened in project tree",
        findTreeItemByText(window, "Box Pressure Demo") != nullptr
    );
    const QStringList populatedTreeGroups{
        zh(u8"几何"),
        zh(u8"面组"),
        zh(u8"材料"),
        zh(u8"边界条件"),
        zh(u8"载荷"),
        zh(u8"网格"),
        zh(u8"结果")
    };
    for (const QString &group : populatedTreeGroups) {
        const int childCount = treeItemChildCount(window, group);
        addStep(
            report,
            "Demo project tree populated: " + group,
            childCount > 0,
            QString("children=%1").arg(childCount)
        );
    }
    const QStringList styledTreeGroups{
        zh(u8"几何"),
        zh(u8"面组"),
        zh(u8"材料"),
        zh(u8"边界条件"),
        zh(u8"载荷"),
        zh(u8"网格"),
        zh(u8"求解器"),
        zh(u8"结果")
    };
    for (const QString &group : styledTreeGroups) {
        addStep(report, "Project tree category styled: " + group, treeItemHasCategoryStyle(window, group));
    }
    addActionEnabledStep(report, window, "geometry.create.box");
    addActionEnabledStep(report, window, "geometry.create.cylinder");
    addActionEnabledStep(report, window, "solverData.create.material");
    addActionEnabledStep(report, window, "solverData.create.boundaryCondition");
    addActionEnabledStep(report, window, "solverData.create.load");
    addActionEnabledStep(report, window, "solver.run.calculix");

    QAction *projectResourcesAction = findActionByText(window, zh(u8"工程资源"));
    addStep(
        report,
        "Project Resources enabled after demo open",
        projectResourcesAction && projectResourcesAction->isEnabled(),
        projectResourcesAction ? QString() : "Action not found"
    );
    const bool geometrySelected = selectTreeItem(window, "Box_1");
    addStep(report, "Demo geometry selection works", geometrySelected);
    if (geometrySelected) {
        addActionEnabledStep(report, window, "mesh.generate");
        addStep(
            report,
            "Property panel selection updated after geometry selection",
            namedLabelText(window, "property.name.value") == "Box_1",
            namedLabelText(window, "property.name.value")
        );
    }
    const QStringList propertySections{
        "property.section.selection",
        "property.section.geometry",
        "property.section.sourceMesh",
        "property.section.details"
    };
    for (const QString &section : propertySections) {
        addStep(report, "Property panel section exists: " + section, hasNamedWidget(window, section));
    }
    addStep(
        report,
        "Property panel is vertically scrollable",
        hasResizableScrollArea(window, "property.scrollArea")
    );

    addStep(
        report,
        "Result field control disabled before result selection",
        !resultFieldControlEnabled(window)
    );
    const bool resultSelected = selectTreeItem(window, "CalculiX Result - Box Pressure Demo");
    addStep(report, "Demo result selection works", resultSelected);
    if (resultSelected) {
        QAction *resultFieldAction = findActionByText(window, "Ux");
        addStep(
            report,
            "Result field actions enabled after result selection",
            resultFieldAction && resultFieldAction->isEnabled(),
            resultFieldAction ? QString() : "Action not found"
        );
        addStep(
            report,
            "Result field control enabled after result selection",
            resultFieldControlEnabled(window)
        );
        const QStringList resultSections{
            "result.section.identity",
            "result.section.display",
            "result.section.probe",
            "result.section.extremes",
            "result.section.status",
            "result.section.animation",
            "result.section.exportHistory"
        };
        for (const QString &section : resultSections) {
            addStep(report, "Result postprocess section exists: " + section, hasNamedWidget(window, section));
        }
        addStep(
            report,
            "Result postprocess panel is vertically scrollable",
            hasResizableScrollArea(window, "resultPostprocess.scrollArea")
        );
        addStep(
            report,
            "Result postprocess field control enabled",
            namedComboBoxEnabled(window, "result.field.combo")
        );
        addStep(
            report,
            "Result postprocess export enabled",
            namedButtonEnabled(window, "result.export.csv")
                && namedButtonEnabled(window, "result.export.report")
                && namedButtonEnabled(window, "result.export.screenshot")
        );
        addStep(
            report,
            "Result postprocess status populated",
            namedLabelText(window, "result.coverage.label").startsWith(QString::fromUtf8(u8"覆盖率：节点")),
            namedLabelText(window, "result.coverage.label")
        );
        addStep(
            report,
            "Result postprocess field unit populated",
            namedLabelText(window, "result.fieldUnit.label").contains(QString::fromUtf8(u8"单位：model length")),
            namedLabelText(window, "result.fieldUnit.label")
        );
        addStep(report, "Result scalar range lock exists", hasNamedWidget(window, "result.scalarRange.lock"));
        addStep(report, "Result scalar range min exists", hasNamedWidget(window, "result.scalarRange.min"));
        addStep(report, "Result scalar range max exists", hasNamedWidget(window, "result.scalarRange.max"));
        addStep(
            report,
            "Result probe prompt visible",
            namedLabelText(window, "result.probe.label").contains(QString::fromUtf8(u8"点击结果模型")),
            namedLabelText(window, "result.probe.label")
        );
    }
    window.prepareForAutomationShutdown();
}
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
    {
        qApp->setProperty("mycae.skipVtkCanvas", true);
        MainWindow *window = new MainWindow;
        for (QWidget *widget : window->findChildren<QWidget *>()) {
            widget->setProperty("mycae.skipResultRender", true);
        }
        const QStringList menuTitles{
            zh(u8"文件"),
            zh(u8"编辑"),
            zh(u8"几何"),
            zh(u8"工况"),
            zh(u8"仿真"),
            zh(u8"结果"),
            zh(u8"工具")
        };
        for (const QString &title : menuTitles) {
            addStep(report, "Menu exists: " + title, findTopLevelMenu(*window, title) != nullptr);
        }
        addStep(
            report,
            "Top-level menu count is compact",
            topLevelMenuCount(*window) == menuTitles.size(),
            QString("count=%1").arg(topLevelMenuCount(*window))
        );
        const QStringList removedTopLevelMenus{
            "Picking",
            "Solver Setup",
            "Mesh",
            "Postprocess"
        };
        for (const QString &title : removedTopLevelMenus) {
            addStep(report, "Old top-level menu removed: " + title, findTopLevelMenu(*window, title) == nullptr);
        }
        QMenu *fileMenu = findTopLevelMenu(*window, zh(u8"文件"));
        QMenu *recentProjectsMenu = findSubMenu(fileMenu, zh(u8"最近工程"));
        addStep(report, "Recent Projects submenu exists", recentProjectsMenu != nullptr);
        addStep(
            report,
            "Recent Projects submenu has 8 slots",
            countActionsBeforeFirstSeparator(recentProjectsMenu) == 8,
            recentProjectsMenu ? QString("slots=%1").arg(countActionsBeforeFirstSeparator(recentProjectsMenu)) : "Menu not found"
        );
        QAction *clearRecentProjectsAction = findMenuActionByText(recentProjectsMenu, zh(u8"清空最近工程"));
        addStep(
            report,
            "Clear Recent Projects is disabled initially",
            clearRecentProjectsAction && !clearRecentProjectsAction->isEnabled(),
            clearRecentProjectsAction ? QString() : "Action not found"
        );
        QMenu *geometryMenu = findTopLevelMenu(*window, zh(u8"几何"));
        addStep(report, "Geometry Face Groups submenu exists", findSubMenu(geometryMenu, zh(u8"面组")) != nullptr);
        QMenu *resultsMenu = findTopLevelMenu(*window, zh(u8"结果"));
        addStep(report, "Results field submenu exists", findSubMenu(resultsMenu, zh(u8"结果场")) != nullptr);
        addStep(report, "Results deformation submenu exists", findSubMenu(resultsMenu, zh(u8"变形比例")) != nullptr);

        const QStringList dockTitles{
            zh(u8"工程 / 模型"),
            zh(u8"诊断"),
            zh(u8"属性"),
            zh(u8"结果后处理"),
            zh(u8"日志")
        };
        for (const QString &title : dockTitles) {
            addStep(report, "Dock exists: " + title, hasDockWidgetTitle(*window, title));
        }
    addStep(report, "Diagnostics table exists", hasNamedWidget(*window, "diagnostic.table"));
    addStep(report, "Diagnostics empty state visible", diagnosticEmptyStateVisible(*window));
    addStep(report, "Log view exists", hasNamedWidget(*window, "log.view"));
    addStep(report, "Log placeholder configured", logPlaceholderConfigured(*window));
        addStep(report, "Toolbar exists: Main Toolbar", hasToolBarTitle(*window, zh(u8"主工具栏")));
        addStep(report, "Toolbar is icon-only", hasIconOnlyToolBar(*window, zh(u8"主工具栏")));
        addStep(report, "Application white UI style applied", qApp && !qApp->styleSheet().isEmpty());
        addStep(report, "Dock style applied", applicationStyleContains("QDockWidget"));
        addStep(report, "Menu style applied", applicationStyleContains("QMenuBar"));
        addStep(report, "Table style applied", applicationStyleContains("QTableWidget"));
        addStep(report, "Input style applied", applicationStyleContains("QLineEdit"));

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
            "solver.run.calculix"
        };
        for (const QString &commandId : requiredCommandIds) {
            addActionExistsStep(report, *window, commandId);
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
            addActionDisabledStep(report, *window, commandId);
        }

        QAction *projectResourcesAction = findActionByText(*window, zh(u8"工程资源"));
        addStep(
            report,
            "Action disabled without project: Project Resources",
            projectResourcesAction && !projectResourcesAction->isEnabled(),
            projectResourcesAction ? QString() : "Action not found"
        );
        QAction *resultFieldAction = findActionByText(*window, "Ux");
        addStep(
            report,
            "Result field actions are disabled without project",
            resultFieldAction && !resultFieldAction->isEnabled(),
            resultFieldAction ? QString() : "Action not found"
        );
        validateDemoProjectUi(report, recentProjectsSettingsGuard, *window);

        window->prepareForAutomationShutdown();
    }
    qApp->setProperty("mycae.skipVtkCanvas", false);

    return report;
}

QString uiValidationStatusName(bool passed)
{
    return passed ? "PASS" : "FAIL";
}
