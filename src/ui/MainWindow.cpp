#include "MainWindow.h"

#include "BoxDialog.h"
#include "LogPanel.h"
#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "RenderView.h"

#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStringList>
#include <QToolBar>

#include <exception>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1200, 760);
    setWindowTitle("MyCAE - Qt 6");

    createActions();
    createMenus();
    createToolBar();
    m_renderView = new RenderView(this);
    setCentralWidget(m_renderView);
    createDockWidgets();

    statusBar()->showMessage("就绪");
    writeLog("MyCAE 已启动，Qt6 桌面界面已就绪。");

}

void MainWindow::createActions()
{
    m_newProjectAction = new QAction("新建工程", this);
    m_newProjectAction->setStatusTip("创建新的 CAE 工程");
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);

    m_openProjectAction = new QAction("打开工程", this);
    m_openProjectAction->setStatusTip("打开已有 CAE 工程");
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::openProject);

    m_createBoxAction = new QAction("创建长方体", this);
    m_createBoxAction->setStatusTip("根据用户输入参数创建长方体几何");
    connect(m_createBoxAction, &QAction::triggered, this, &MainWindow::createBox);

    m_exitAction = new QAction("退出", this);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus()
{
    auto *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    auto *geometryMenu = menuBar()->addMenu("几何");
    geometryMenu->addAction(m_createBoxAction);
    geometryMenu->addAction("导入 STEP", this, [this]() {
        writeLog("已点击导入 STEP。Open CASCADE 集成将在后续阶段实现。");
    });

    auto *simulationMenu = menuBar()->addMenu("仿真");
    simulationMenu->addAction("生成网格", this, [this]() {
        writeLog("已点击生成网格。Gmsh 集成将在后续阶段实现。");
    });
    simulationMenu->addAction("运行求解器", this, [this]() {
        writeLog("已点击运行求解器。CalculiX 集成将在后续阶段实现。");
    });
}

void MainWindow::createToolBar()
{
    auto *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    toolBar->addAction(m_newProjectAction);
    toolBar->addAction(m_openProjectAction);
    toolBar->addSeparator();
    toolBar->addAction(m_createBoxAction);
    toolBar->addAction("网格", this, [this]() {
        writeLog("已点击网格工具。后续阶段将调用 Gmsh。");
    });
    toolBar->addAction("求解", this, [this]() {
        writeLog("已点击求解工具。后续阶段将调用 CalculiX。");
    });
}

void MainWindow::createDockWidgets()
{
    auto *projectDock = new QDockWidget("工程 / 模型", this);
    m_projectTreePanel = new ProjectTreePanel(projectDock);
    connect(m_projectTreePanel, &ProjectTreePanel::geometrySelected, this, &MainWindow::showGeometryProperties);
    projectDock->setWidget(m_projectTreePanel);
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    auto *propertyDock = new QDockWidget("属性", this);
    m_propertyPanel = new PropertyPanel(propertyDock);
    propertyDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propertyDock);

    auto *logDock = new QDockWidget("日志", this);
    m_logPanel = new LogPanel(logDock);
    logDock->setWidget(m_logPanel);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::newProject()
{
    const QString projectPath = QFileDialog::getExistingDirectory(
        this,
        "选择工程目录",
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (projectPath.isEmpty()) {
        writeLog("已取消新建工程。");
        return;
    }

    Project project;
    QString errorMessage;
    if (!m_projectManager.createProject(projectPath, &project, &errorMessage)) {
        QMessageBox::warning(this, "新建工程失败", errorMessage);
        writeLog("新建工程失败：" + errorMessage);
        return;
    }

    setCurrentProject(project);
    m_boxes.clear();
    refreshGeometryTree();
    writeLog("工程已创建：" + project.rootPath);
}

void MainWindow::openProject()
{
    const QString projectFilePath = QFileDialog::getOpenFileName(
        this,
        "打开工程",
        QString(),
        "MyCAE 工程 (project.json);;JSON 文件 (*.json)"
    );

    if (projectFilePath.isEmpty()) {
        writeLog("已取消打开工程。");
        return;
    }

    Project project;
    QString errorMessage;
    if (!m_projectManager.openProject(projectFilePath, &project, &errorMessage)) {
        QMessageBox::warning(this, "打开工程失败", errorMessage);
        writeLog("打开工程失败：" + errorMessage);
        return;
    }

    setCurrentProject(project);
    loadProjectGeometries();
    writeLog("工程已打开：" + project.rootPath);
}

void MainWindow::createBox()
{
    if (m_currentProject.rootPath.isEmpty()) {
        QMessageBox::information(this, "创建长方体", "请先新建或打开工程。");
        writeLog("创建长方体失败：当前没有打开的工程。");
        return;
    }

    BoxDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        writeLog("已取消创建长方体。");
        return;
    }

    BoxGeometry box;
    QString errorMessage;
    if (!m_geometryManager.createBox(m_currentProject, dialog.boxParameters(), &box, &errorMessage)) {
        QMessageBox::warning(this, "创建长方体失败", errorMessage);
        writeLog("创建长方体失败：" + errorMessage);
        return;
    }

    m_boxes.append(box);
    refreshGeometryTree();
    displayBoxGeometry(box);
    if (box.occBrepSaved) {
        writeLog("BREP 保存成功：" + box.occBrepFile);
    } else {
        writeLog("BREP 保存失败：" + box.occBrepErrorMessage);
    }
    if (box.occStepSaved) {
        writeLog("STEP 保存成功：" + box.occStepFile);
    } else {
        writeLog("STEP 保存失败：" + box.occStepErrorMessage);
    }
    writeLog("长方体几何已创建：" + box.name);
}

void MainWindow::setCurrentProject(const Project &project)
{
    m_currentProject = project;
    setWindowTitle("MyCAE - " + project.name);

    if (m_projectTreePanel) {
        m_projectTreePanel->showProject(project.name, project.rootPath);
    }
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }

    statusBar()->showMessage("当前工程：" + project.name);
}

void MainWindow::loadProjectGeometries()
{
    QString errorMessage;
    if (!m_geometryManager.loadBoxGeometries(m_currentProject, &m_boxes, &errorMessage)) {
        QMessageBox::warning(this, "加载几何失败", errorMessage);
        writeLog("加载几何失败：" + errorMessage);
        return;
    }

    refreshGeometryTree();
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    writeLog(QString("已加载 %1 个长方体几何对象。").arg(m_boxes.size()));
}

void MainWindow::refreshGeometryTree()
{
    if (!m_projectTreePanel) {
        return;
    }

    QStringList geometryNames;
    for (const BoxGeometry &box : m_boxes) {
        geometryNames.append(box.name);
    }
    m_projectTreePanel->setGeometryItems(geometryNames);
}

void MainWindow::showGeometryProperties(const QString &geometryName)
{
    for (const BoxGeometry &box : m_boxes) {
        if (box.name == geometryName) {
            displayBoxGeometry(box);
            return;
        }
    }
}

void MainWindow::displayBoxGeometry(const BoxGeometry &box)
{
    if (m_propertyPanel) {
        m_propertyPanel->showBoxGeometry(box);
    }

    if (!m_renderView) {
        return;
    }

    const QString sizeText = QString("%1 %4 x %2 %4 x %3 %4")
        .arg(box.length)
        .arg(box.width)
        .arg(box.height)
        .arg(box.unit);

    const QString brepPath = QFileInfo(box.occBrepFile).isAbsolute()
        ? box.occBrepFile
        : QDir(m_currentProject.rootPath).filePath(box.occBrepFile);

    if (!box.occBrepFile.isEmpty() && QFileInfo::exists(brepPath)) {
        QString errorMessage;
        TopoDS_Shape loadedShape;
        OCCShapeIO shapeIO;
        if (shapeIO.loadBREP(brepPath, loadedShape, &errorMessage)) {
            try {
                m_renderView->showOccShape(loadedShape, box.name, sizeText);
                return;
            } catch (const Standard_Failure &failure) {
                writeLog(QString("BREP 显示失败：%1；将使用参数重建 OCC Shape。").arg(failure.what()));
            } catch (const std::exception &error) {
                writeLog(QString("BREP 显示失败：%1；将使用参数重建 OCC Shape。").arg(error.what()));
            }
        } else {
            writeLog("BREP 加载失败：" + errorMessage + "；将使用参数重建 OCC Shape。");
        }
    } else if (!box.occBrepFile.isEmpty()) {
        writeLog("BREP 文件不存在：" + box.occBrepFile + "；将使用参数重建 OCC Shape。");
    }

    try {
        OCCGeometryFactory factory;
        const TopoDS_Shape shape = factory.createShape(box);
        m_renderView->showOccShape(shape, box.name, sizeText);
    } catch (const Standard_Failure &failure) {
        writeLog(QString("OCC Box 显示失败：%1；已回退到 vtkCubeSource。").arg(failure.what()));
        m_renderView->showBoxGeometry(box);
    } catch (const std::exception &error) {
        writeLog(QString("OCC Box 显示失败：%1；已回退到 vtkCubeSource。").arg(error.what()));
        m_renderView->showBoxGeometry(box);
    }
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
}
