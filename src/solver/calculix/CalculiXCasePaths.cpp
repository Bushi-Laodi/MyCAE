#include "solver/calculix/CalculiXCasePaths.h"

#include "solver/SimulationCase.h"

#include <QDir>

namespace
{
QString sanitizedFileStem(QString value)
{
    value = value.trimmed();
    bool hasUsefulCharacter = false;

    for (QChar &ch : value) {
        if (ch.isLetterOrNumber()) {
            hasUsefulCharacter = true;
        } else if (ch.isSpace() || ch == '_' || ch == '-') {
            ch = '_';
        } else {
            ch = '_';
        }
    }

    return hasUsefulCharacter ? value : QString();
}

QString calculixJobName(QString stem)
{
    stem = sanitizedFileStem(stem);
    if (stem.isEmpty()) {
        stem = "simulation_case";
    }
    return stem.endsWith("_calculix", Qt::CaseInsensitive) ? stem : stem + "_calculix";
}

QString preferredCaseName(const SolverCaseContext &context)
{
    if (context.simulationCase) {
        const QString name = sanitizedFileStem(context.simulationCase->name);
        if (!name.isEmpty()) {
            return name;
        }

        const QString id = sanitizedFileStem(context.simulationCase->id);
        if (!id.isEmpty()) {
            return id;
        }
    }

    const QString contextName = sanitizedFileStem(context.caseName);
    return contextName.isEmpty() ? QString("simulation_case") : contextName;
}
}

CalculiXCasePaths CalculiXCasePathsBuilder::fromContext(const SolverCaseContext &context)
{
    CalculiXCasePaths paths;
    paths.caseDirectory = context.caseDirectory;
    paths.jobName = calculixJobName(preferredCaseName(context));

    const QDir caseDir(paths.caseDirectory);
    paths.inputFile = caseDir.filePath(paths.jobName + ".inp");
    paths.datFile = caseDir.filePath(paths.jobName + ".dat");
    paths.staFile = caseDir.filePath(paths.jobName + ".sta");
    paths.frdFile = caseDir.filePath(paths.jobName + ".frd");
    paths.logFile = caseDir.filePath(paths.jobName + ".log");
    return paths;
}
