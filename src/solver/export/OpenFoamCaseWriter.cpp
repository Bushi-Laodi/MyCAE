#include "solver/export/OpenFoamCaseWriter.h"

#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "solver/SimulationCase.h"
#include "solver/openfoam/OpenFoamServiceClient.h"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
const MeshObject *caseMesh(const SolverCaseContext &context)
{
    if (!context.projectModel || !context.simulationCase) {
        return nullptr;
    }
    return context.projectModel->findMeshByName(context.simulationCase->meshName);
}

bool writeManifest(const QString &filePath, const QJsonObject &manifest, QStringList &errors)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        errors.append("Cannot write OpenFOAM request manifest: " + file.errorString());
        return false;
    }
    file.write(QJsonDocument(manifest).toJson(QJsonDocument::Indented));
    return true;
}
}

SolverCaseWriterResult OpenFoamCaseWriter::write(const SolverCaseContext &context) const
{
    SolverCaseWriterResult result;
    if (!context.isValid()) {
        result.errors.append("OpenFOAM export failed: invalid solver case context.");
        return result;
    }

    if (!QDir().mkpath(context.caseDirectory)) {
        result.errors.append("OpenFOAM export failed: cannot create case directory: " + context.caseDirectory);
        return result;
    }

    const SimulationCase &simulationCase = *context.simulationCase;
    const CfdCase &cfdCase = simulationCase.cfdCase;
    const MeshObject *mesh = caseMesh(context);

    QJsonObject manifest;
    manifest.insert("solver", "openfoam");
    manifest.insert("stage", "fastapi-demo");
    manifest.insert("caseName", context.caseName);
    manifest.insert("caseDirectory", QDir::toNativeSeparators(context.caseDirectory));
    manifest.insert("serviceUrl", OpenFoamServiceClient::defaultBaseUrl().toString(QUrl::StripTrailingSlash));
    manifest.insert("meshName", cfdCase.meshName.trimmed().isEmpty() ? simulationCase.meshName : cfdCase.meshName);
    manifest.insert("meshNodeCount", mesh ? mesh->nodeCount : 0);
    manifest.insert("meshCellCount", mesh ? mesh->tetraCount : 0);
    manifest.insert("materialCount", static_cast<int>(cfdCase.materials.size()));
    manifest.insert("boundaryConditionCount", static_cast<int>(cfdCase.boundaries.size()));
    manifest.insert("loadCount", static_cast<int>(cfdCase.fieldValues.size()));

    const QString manifestPath = OpenFoamServiceClient::requestManifestPath(context.caseDirectory);
    if (!writeManifest(manifestPath, manifest, result.errors)) {
        return result;
    }

    result.success = true;
    result.caseRootPath = context.caseDirectory;
    result.writtenFiles.append(manifestPath);
    result.logMessages.append("OpenFOAM demo request exported: " + manifestPath);
    result.logMessages.append("OpenFOAM service URL: " + manifest.value("serviceUrl").toString());
    return result;
}
