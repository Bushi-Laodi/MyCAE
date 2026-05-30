#pragma once

#include <QVector>

#include <functional>

class QAction;
class QMainWindow;
class QMenu;
class QString;

class ActionRegistry;
class UndoStackController;
struct WorkflowCommandContext;

struct MainWindowActions
{
    QAction *newProject = nullptr;
    QAction *openProject = nullptr;
    QMenu *recentProjectsMenu = nullptr;
    QVector<QAction *> recentProjects;
    QAction *clearRecentProjects = nullptr;
    QAction *undo = nullptr;
    QAction *redo = nullptr;
    QAction *createBox = nullptr;
    QAction *createCylinder = nullptr;
    QAction *createSphere = nullptr;
    QAction *importStep = nullptr;
    QAction *createBoolean = nullptr;
    QAction *transformGeometry = nullptr;
    QAction *deleteGeometry = nullptr;
    QAction *showGeometryEdges = nullptr;
    QAction *showOrientationMarker = nullptr;
    QAction *checkGmsh = nullptr;
    QAction *generateMesh = nullptr;
    QAction *readMeshInfo = nullptr;
    QAction *showMesh = nullptr;
    QAction *showMeshTransparent = nullptr;
    QAction *pickFace = nullptr;
    QAction *clearPick = nullptr;
    QAction *createFaceGroupFromPick = nullptr;
    QAction *addPickedFacesToFaceGroup = nullptr;
    QAction *removePickedFacesFromFaceGroup = nullptr;
    QAction *clearFaceGroupFaces = nullptr;
    QAction *renameFaceGroup = nullptr;
    QAction *deleteFaceGroup = nullptr;
    QAction *setFaceGroupLocalMeshSize = nullptr;
    QAction *toggleFaceGroupPhysicalGroup = nullptr;
    QAction *exit = nullptr;
    QVector<QAction *> runSolvers;

    QAction *createMaterial = nullptr;
    QAction *createStructuralMaterial = nullptr;
    QAction *createFluidMaterial = nullptr;
    QAction *createSectionAssignment = nullptr;
    QAction *createBoundaryCondition = nullptr;
    QAction *createStructuralBoundaryCondition = nullptr;
    QAction *createCfdBoundaryCondition = nullptr;
    QAction *createLoad = nullptr;
    QAction *createStructuralLoad = nullptr;
    QAction *createCfdFieldValue = nullptr;
    QAction *editSolverData = nullptr;
    QAction *deleteSolverData = nullptr;
    QVector<QAction *> resultFields;
    QVector<QAction *> resultScales;
    QAction *exportScreenshot = nullptr;
    QAction *projectResources = nullptr;
    QAction *validateSamples = nullptr;
    QAction *clearDiagnostics = nullptr;
};

struct MainWindowActionCallbacks
{
    std::function<void(const QString &)> openRecentProject;
    std::function<void()> clearRecentProjects;
    std::function<void(const QString &)> setSelectedResultField;
    std::function<void(double)> setSelectedResultDeformationScale;
    std::function<void()> exportRenderScreenshot;
    std::function<void()> showProjectResources;
    std::function<void()> validateSamples;
    std::function<void()> clearDiagnostics;
};

class MainWindowActionBuilder
{
public:
    static MainWindowActions build(
        QMainWindow *window,
        ActionRegistry &actionRegistry,
        const WorkflowCommandContext &context,
        UndoStackController &undoStackController,
        const MainWindowActionCallbacks &callbacks
    );
};
