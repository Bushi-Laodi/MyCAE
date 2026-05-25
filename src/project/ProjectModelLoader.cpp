#include "ProjectModelLoader.h"

#include "mesh/MeshManager.h"
#include "mesh/MeshBoundaryBuilder.h"
#include "mesh/MeshData.h"
#include "mesh/MshReader.h"
#include "project/ProjectModel.h"
#include "result/ResultManager.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/SimulationCaseManager.h"

#include <QDir>
#include <QDateTime>
#include <QFileInfo>

#include <vector>

namespace
{
QStringList resultFieldNames()
{
    return QStringList{
        CalculiXResultFields::Ux,
        CalculiXResultFields::Uy,
        CalculiXResultFields::Uz,
        CalculiXResultFields::DisplacementMagnitude,
        CalculiXResultFields::VonMisesStress
    };
}

QString firstFileWithSuffix(const QFileInfoList &files, const QString &suffix)
{
    for (const QFileInfo &file : files) {
        if (file.suffix().compare(suffix, Qt::CaseInsensitive) == 0) {
            return file.absoluteFilePath();
        }
    }
    return {};
}

std::vector<ResultObject> recoverCalculiXResultsFromDisk(const ProjectModel &projectModel)
{
    std::vector<ResultObject> results;
    const QDir calculixRoot(QDir(projectModel.project().rootPath).filePath("solver/calculix"));
    if (!calculixRoot.exists()) {
        return results;
    }

    QString fallbackMeshName;
    if (projectModel.meshObjects().size() == 1) {
        fallbackMeshName = projectModel.meshObjects().front().name;
    }

    const QFileInfoList directories = calculixRoot.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);
    for (const QFileInfo &directory : directories) {
        const QDir caseDir(directory.absoluteFilePath());
        const QFileInfoList files = caseDir.entryInfoList(QDir::Files, QDir::Name);
        const QString datFile = firstFileWithSuffix(files, "dat");
        if (datFile.isEmpty()) {
            continue;
        }

        ResultObject result;
        result.id = "calculix_recovered_" + directory.fileName();
        result.name = "CalculiX Result - " + directory.fileName();
        result.solverName = "CalculiX";
        result.meshName = fallbackMeshName;
        result.casePath = directory.absoluteFilePath();
        result.datFile = datFile;
        result.frdFile = firstFileWithSuffix(files, "frd");
        result.staFile = firstFileWithSuffix(files, "sta");
        result.logFile = firstFileWithSuffix(files, "log");
        for (const QFileInfo &file : files) {
            const QString suffix = file.suffix().toLower();
            if (suffix == "dat" || suffix == "frd" || suffix == "sta" || suffix == "log") {
                result.resultFiles.append(file.absoluteFilePath());
            }
        }
        result.availableFields = resultFieldNames();
        result.primaryFieldName = CalculiXResultFields::DisplacementMagnitude;
        result.displayFieldName = result.primaryFieldName;
        result.deformationScale = 0.0;
        result.createdAt = directory.lastModified().toString(Qt::ISODate);
        result.success = true;
        result.summary = QString("Recovered CalculiX result from disk: files=%1").arg(result.resultFiles.size());
        results.push_back(result);
    }
    return results;
}
}

ProjectModelLoader::ProjectModelLoader(const GeometryManager &geometryManager)
    : m_geometryManager(geometryManager)
{
}

bool ProjectModelLoader::loadGeometries(ProjectModel &projectModel, QString *errorMessage) const
{
    QVector<BoxGeometry> loadedBoxes;
    if (!m_geometryManager.loadBoxGeometries(projectModel.project(), &loadedBoxes, errorMessage)) {
        return false;
    }

    QVector<CylinderGeometry> loadedCylinders;
    if (!m_geometryManager.loadCylinderGeometries(projectModel.project(), &loadedCylinders, errorMessage)) {
        return false;
    }

    std::vector<GeometryObject> loadedGeometries;
    if (!m_geometryManager.loadGeometryObjects(projectModel.project(), loadedGeometries, errorMessage)) {
        return false;
    }

    GeometryRepository &geometryRepository = projectModel.geometryRepository();
    geometryRepository.boxes() = loadedBoxes;
    geometryRepository.cylinders() = loadedCylinders;
    geometryRepository.geometryObjects().clear();
    for (const GeometryObject &geometry : loadedGeometries) {
        geometryRepository.geometryObjects().append(geometry);
    }
    projectModel.ensureDefaultFaceGroups();

    projectModel.clearSelectionIfKind(SelectionKind::Geometry);
    return true;
}

bool ProjectModelLoader::loadMeshes(ProjectModel &projectModel, QString *errorMessage) const
{
    MeshManager meshManager(projectModel.project().rootPath);
    std::vector<MeshObject> loadedMeshes;
    if (!meshManager.loadMeshObjects(loadedMeshes, errorMessage)) {
        return false;
    }

    MeshRepository &meshRepository = projectModel.meshRepository();
    meshRepository.meshObjects().clear();
    meshRepository.meshBoundaries().clear();
    for (const MeshObject &meshObject : loadedMeshes) {
        meshRepository.meshObjects().append(meshObject);

        const QString meshPath = QFileInfo(meshObject.mshFile).isAbsolute()
            ? meshObject.mshFile
            : QDir(projectModel.project().rootPath).filePath(meshObject.mshFile);
        MeshData meshData;
        QString meshReadError;
        if (QFileInfo::exists(meshPath) && MshReader::readMsh2(meshPath, meshData, &meshReadError)) {
            meshRepository.replaceMeshBoundariesForMesh(meshObject.name, MeshBoundaryBuilder::build(meshData, meshObject));
        }
    }

    projectModel.clearSelectionIfKind(SelectionKind::Mesh);
    return true;
}

bool ProjectModelLoader::loadSimulationCase(ProjectModel &projectModel, QString *errorMessage) const
{
    SimulationCaseManager simulationCaseManager;
    if (!simulationCaseManager.exists(projectModel.project())) {
        projectModel.solverRepository().clearSolverData();
        projectModel.ensureDefaultFaceGroups();
        return true;
    }

    SimulationCase simulationCase;
    if (!simulationCaseManager.load(projectModel.project(), simulationCase, errorMessage)) {
        return false;
    }

    SolverRepository &solverRepository = projectModel.solverRepository();
    solverRepository.materials() = simulationCase.materials;
    solverRepository.boundaryConditions() = simulationCase.boundaryConditions;
    solverRepository.loads() = simulationCase.loads;
    solverRepository.faceGroups() = simulationCase.geometrySetup.faceGroups;
    projectModel.ensureDefaultFaceGroups();
    return true;
}

bool ProjectModelLoader::loadResults(ProjectModel &projectModel, QString *errorMessage) const
{
    ResultManager resultManager;
    projectModel.resultRepository().clear();
    if (!resultManager.exists(projectModel.project())) {
        projectModel.resultRepository().results() = recoverCalculiXResultsFromDisk(projectModel);
        projectModel.clearSelectionIfKind(SelectionKind::Result);
        return true;
    }

    std::vector<ResultObject> loadedResults;
    if (!resultManager.load(projectModel.project(), loadedResults, errorMessage)) {
        return false;
    }

    projectModel.resultRepository().results() = loadedResults;
    projectModel.clearSelectionIfKind(SelectionKind::Result);
    return true;
}
