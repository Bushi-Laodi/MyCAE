#include "solver/SimulationCaseManager.h"

#include "project/ProjectModel.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/SimulationCaseJsonReader.h"
#include "solver/SimulationCaseJsonWriter.h"

#include <QDir>
#include <QFileInfo>

QString SimulationCaseManager::relativeCaseFilePath()
{
    return QDir("solver").filePath("case.json");
}

QString SimulationCaseManager::caseFilePath(const Project &project)
{
    return QDir(project.rootPath).filePath(relativeCaseFilePath());
}

bool SimulationCaseManager::save(const ProjectModel &projectModel, QString *errorMessage) const
{
    if (!projectModel.hasProject()) {
        if (errorMessage) {
            *errorMessage = "Cannot save simulation case without an active project.";
        }
        return false;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(projectModel);
    return SimulationCaseJsonWriter::writeCaseFile(
        simulationCase,
        caseFilePath(projectModel.project()),
        errorMessage
    );
}

bool SimulationCaseManager::load(
    const Project &project,
    SimulationCase &simulationCase,
    QString *errorMessage
) const
{
    return SimulationCaseJsonReader::readCaseFile(caseFilePath(project), simulationCase, errorMessage);
}

bool SimulationCaseManager::exists(const Project &project) const
{
    return QFileInfo::exists(caseFilePath(project));
}
