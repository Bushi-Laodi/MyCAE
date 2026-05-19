#include "MainWindow.h"

#include "BoxDialog.h"
#include "LogPanel.h"
#include "mesh/GmshRunner.h"
#include "mesh/MeshManager.h"
#include "mesh/MeshToVtkConverter.h"
#include "mesh/MshReader.h"
#include "occ/OCCGeometryFactory.h"
#include "occ/OCCShapeIO.h"
#include "ProjectTreePanel.h"
#include "PropertyPanel.h"
#include "RenderView.h"

#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>
#include <vtkUnstructuredGrid.h>

#include <QAction>
#include <QDateTime>
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

    m_checkGmshAction = new QAction("Check Gmsh", this);
    m_checkGmshAction->setStatusTip("Run gmsh.exe --version to check the local Gmsh environment");
    connect(m_checkGmshAction, &QAction::triggered, this, &MainWindow::checkGmsh);

    m_generateMeshAction = new QAction("Generate Mesh", this);
    m_generateMeshAction->setStatusTip("Generate a minimal 3D mesh from the selected STEP geometry");
    connect(m_generateMeshAction, &QAction::triggered, this, &MainWindow::generateMesh);

    m_readMeshInfoAction = new QAction("Read Mesh Info", this);
    m_readMeshInfoAction->setStatusTip("Read node and tetrahedron counts from the selected MSH file");
    connect(m_readMeshInfoAction, &QAction::triggered, this, &MainWindow::readMeshInfo);

    m_showMeshAction = new QAction("Show Mesh", this);
    m_showMeshAction->setStatusTip("Read and display the selected tetrahedral MSH file");
    connect(m_showMeshAction, &QAction::triggered, this, &MainWindow::showMesh);

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

    auto *meshMenu = menuBar()->addMenu("Mesh");
    meshMenu->addAction(m_checkGmshAction);
    meshMenu->addAction(m_generateMeshAction);
    meshMenu->addAction(m_readMeshInfoAction);
    meshMenu->addAction(m_showMeshAction);

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
    connect(m_projectTreePanel, &ProjectTreePanel::meshSelected, this, &MainWindow::showMeshObject);
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
    m_meshObjects.clear();
    m_selectedBoxIndex = -1;
    m_selectedMeshIndex = -1;
    refreshGeometryTree();
    refreshMeshTree();
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
    loadProjectMeshes();
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
    m_selectedBoxIndex = m_boxes.size() - 1;
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

void MainWindow::checkGmsh()
{
    const GmshRunner gmshRunner;
    const GmshRunResult result = gmshRunner.checkVersion();

    writeLog("Gmsh 路径：" + gmshRunner.gmshExecutablePath());
    writeLog("Gmsh 命令：" + gmshRunner.gmshExecutablePath() + " --version");
    writeLog(QString("Gmsh exitCode：%1").arg(result.exitCode));
    writeLog("Gmsh stdout：" + (result.standardOutput.isEmpty() ? QString("<empty>") : result.standardOutput));
    writeLog("Gmsh stderr：" + (result.standardError.isEmpty() ? QString("<empty>") : result.standardError));

    if (result.success) {
        writeLog("Gmsh 环境检查成功。");
    } else {
        writeLog(result.errorMessage.isEmpty() ? "Gmsh 环境检查失败：未知错误。" : result.errorMessage);
    }
}

void MainWindow::generateMesh()
{
    if (m_currentProject.rootPath.isEmpty()) {
        writeLog("网格生成失败：请先新建或打开工程。");
        return;
    }

    if (m_selectedBoxIndex < 0 || m_selectedBoxIndex >= m_boxes.size()) {
        writeLog("请先在工程树中选择一个几何对象。");
        return;
    }

    const BoxGeometry &box = m_boxes.at(m_selectedBoxIndex);
    if (box.occStepFile.isEmpty()) {
        writeLog("当前几何对象没有 STEP 文件，无法生成网格。");
        return;
    }

    const QString stepAbsPath = QFileInfo(box.occStepFile).isAbsolute()
        ? box.occStepFile
        : QDir(m_currentProject.rootPath).filePath(box.occStepFile);
    const QString meshRelativePath = QDir("mesh").filePath(QFileInfo(stepAbsPath).completeBaseName() + ".msh");
    const QString meshAbsPath = QDir(m_currentProject.rootPath).filePath(meshRelativePath);

    const GmshRunner gmshRunner;
    const GmshRunResult result = gmshRunner.generate3DMesh(stepAbsPath, meshAbsPath);

    writeLog("Gmsh 路径：" + gmshRunner.gmshExecutablePath());
    writeLog("Gmsh 命令：" + gmshRunner.gmshExecutablePath()
             + " " + stepAbsPath
             + " -3 -format msh2 -o " + meshAbsPath);
    writeLog("Gmsh 输入文件：" + stepAbsPath);
    writeLog("Gmsh 输出文件：" + meshAbsPath);
    writeLog(QString("Gmsh exitCode：%1").arg(result.exitCode));
    writeLog("Gmsh stdout：" + (result.standardOutput.isEmpty() ? QString("<empty>") : result.standardOutput));
    writeLog("Gmsh stderr：" + (result.standardError.isEmpty() ? QString("<empty>") : result.standardError));

    if (result.success) {
        writeLog("网格生成成功：" + meshRelativePath);
        MeshData meshData;
        QString errorMessage;
        if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
            writeLog("MeshObject 保存失败：读取新生成的 MSH 失败：" + errorMessage);
            return;
        }

        MeshObject meshObject;
        meshObject.name = box.name + "_Mesh";
        meshObject.sourceGeometryName = box.name;
        meshObject.mshFile = meshRelativePath;
        meshObject.type = "tetra4";
        meshObject.nodeCount = meshData.nodeCount();
        meshObject.tetraCount = meshData.tetraCount();
        meshObject.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);

        MeshManager meshManager(m_currentProject.rootPath);
        if (!meshManager.saveMeshObject(meshObject, &errorMessage)) {
            writeLog("MeshObject 保存失败：" + errorMessage);
            return;
        }

        bool replaced = false;
        for (int meshIndex = 0; meshIndex < m_meshObjects.size(); ++meshIndex) {
            MeshObject &existingMesh = m_meshObjects[meshIndex];
            if (existingMesh.sourceGeometryName == meshObject.sourceGeometryName) {
                existingMesh = meshObject;
                m_selectedMeshIndex = meshIndex;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            m_meshObjects.append(meshObject);
            m_selectedMeshIndex = m_meshObjects.size() - 1;
        }
        refreshMeshTree();
        writeLog("MeshObject 保存成功：mesh/" + box.name.toLower() + "_mesh.json");
    } else {
        writeLog(result.errorMessage.isEmpty() ? "网格生成失败：未知错误。" : result.errorMessage);
    }
}

void MainWindow::readMeshInfo()
{
    if (m_currentProject.rootPath.isEmpty()) {
        writeLog("读取网格失败：请先新建或打开工程。");
        return;
    }

    if (m_selectedBoxIndex < 0 || m_selectedBoxIndex >= m_boxes.size()) {
        writeLog("请先在工程树中选择一个几何对象。");
        return;
    }

    const BoxGeometry &box = m_boxes.at(m_selectedBoxIndex);
    const QString meshRelativePath = QDir("mesh").filePath(box.name.toLower() + ".msh");
    const QString meshAbsPath = QDir(m_currentProject.rootPath).filePath(meshRelativePath);

    writeLog("MSH 文件路径：" + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("读取网格失败：MSH 文件不存在。");
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = box.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("读取网格失败：" + errorMessage);
        return;
    }

    meshData.sourceGeometryName = box.name;
    writeLog("读取网格成功。");
    writeLog(QString("网格节点数量：%1").arg(meshData.nodeCount()));
    writeLog(QString("四面体单元数量：%1").arg(meshData.tetraCount()));
}

void MainWindow::showMesh()
{
    if (m_currentProject.rootPath.isEmpty()) {
        writeLog("显示网格失败：请先新建或打开工程。");
        return;
    }

    if (m_selectedBoxIndex < 0 || m_selectedBoxIndex >= m_boxes.size()) {
        writeLog("请先在工程树中选择一个几何对象。");
        return;
    }

    const BoxGeometry &box = m_boxes.at(m_selectedBoxIndex);
    const QString meshRelativePath = QDir("mesh").filePath(box.name.toLower() + ".msh");
    const QString meshAbsPath = QDir(m_currentProject.rootPath).filePath(meshRelativePath);

    writeLog("MSH 文件路径：" + meshAbsPath);
    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("显示网格失败：MSH 文件不存在。");
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = box.name;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("显示网格失败：读取 MSH 失败：" + errorMessage);
        return;
    }
    meshData.sourceGeometryName = box.name;

    writeLog(QString("网格节点数量：%1").arg(meshData.nodeCount()));
    writeLog(QString("四面体单元数量：%1").arg(meshData.tetraCount()));

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        writeLog("VTK 网格转换失败：" + errorMessage);
        return;
    }
    writeLog("VTK 网格转换成功。");

    if (!m_renderView) {
        writeLog("网格显示失败：渲染窗口不可用。");
        return;
    }

    const QString subtitle = QString("%1 nodes, %2 tetrahedra")
        .arg(meshData.nodeCount())
        .arg(meshData.tetraCount());
    m_renderView->showMeshGrid(grid, box.name + " Mesh", subtitle);
    writeLog("网格显示成功。");
}

void MainWindow::setCurrentProject(const Project &project)
{
    m_currentProject = project;
    m_selectedBoxIndex = -1;
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
    m_selectedBoxIndex = -1;
    if (m_propertyPanel) {
        m_propertyPanel->showEmptySelection();
    }
    if (m_renderView) {
        m_renderView->showEmpty();
    }
    writeLog(QString("已加载 %1 个长方体几何对象。").arg(m_boxes.size()));
}

void MainWindow::loadProjectMeshes()
{
    m_meshObjects.clear();

    MeshManager meshManager(m_currentProject.rootPath);
    std::vector<MeshObject> loadedMeshes;
    QString errorMessage;
    if (!meshManager.loadMeshObjects(loadedMeshes, &errorMessage)) {
        QMessageBox::warning(this, "加载网格失败", errorMessage);
        writeLog("加载网格失败：" + errorMessage);
        return;
    }

    for (const MeshObject &meshObject : loadedMeshes) {
        m_meshObjects.append(meshObject);
    }

    refreshMeshTree();
    m_selectedMeshIndex = -1;
    writeLog(QString("已加载 %1 个网格对象。").arg(m_meshObjects.size()));
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

void MainWindow::refreshMeshTree()
{
    if (!m_projectTreePanel) {
        return;
    }

    QStringList meshNames;
    for (const MeshObject &meshObject : m_meshObjects) {
        meshNames.append(meshObject.name);
    }
    m_projectTreePanel->setMeshItems(meshNames);
}

void MainWindow::showGeometryProperties(const QString &geometryName)
{
    for (int index = 0; index < m_boxes.size(); ++index) {
        const BoxGeometry &box = m_boxes.at(index);
        if (box.name == geometryName) {
            m_selectedBoxIndex = index;
            m_selectedMeshIndex = -1;
            displayBoxGeometry(box);
            return;
        }
    }
    m_selectedBoxIndex = -1;
}

void MainWindow::showMeshObject(const QString &meshName)
{
    for (int index = 0; index < m_meshObjects.size(); ++index) {
        const MeshObject &meshObject = m_meshObjects.at(index);
        if (meshObject.name == meshName) {
            m_selectedMeshIndex = index;
            m_selectedBoxIndex = -1;
            displayMeshObject(meshObject);
            return;
        }
    }
    m_selectedMeshIndex = -1;
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

void MainWindow::displayMeshObject(const MeshObject &meshObject)
{
    if (m_propertyPanel) {
        m_propertyPanel->showMeshObject(meshObject);
    }

    if (!m_renderView) {
        return;
    }

    const QString meshAbsPath = QFileInfo(meshObject.mshFile).isAbsolute()
        ? meshObject.mshFile
        : QDir(m_currentProject.rootPath).filePath(meshObject.mshFile);

    if (!QFileInfo::exists(meshAbsPath)) {
        writeLog("显示网格失败：MSH 文件不存在：" + meshObject.mshFile);
        return;
    }

    MeshData meshData;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    QString errorMessage;
    if (!MshReader::readMsh2(meshAbsPath, meshData, &errorMessage)) {
        writeLog("显示网格失败：读取 MSH 失败：" + errorMessage);
        return;
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = MeshToVtkConverter::toUnstructuredGrid(meshData, &errorMessage);
    if (!grid) {
        writeLog("VTK 网格转换失败：" + errorMessage);
        return;
    }

    const QString subtitle = QString("%1 nodes, %2 tetrahedra")
        .arg(meshData.nodeCount())
        .arg(meshData.tetraCount());
    m_renderView->showMeshGrid(grid, meshObject.name, subtitle);
    writeLog("网格显示成功：" + meshObject.name);
}

void MainWindow::writeLog(const QString &message)
{
    if (m_logPanel) {
        m_logPanel->appendMessage(message);
    }
}
