#include "solver/openfoam/OpenFoamPlugin.h"

#include "solver/export/OpenFoamCaseWriter.h"
#include "solver/openfoam/OpenFoamServiceClient.h"

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
QStringList jsonStringArray(const QJsonValue &value)
{
    QStringList result;
    const QJsonArray array = value.toArray();
    for (const QJsonValue &item : array) {
        const QString text = item.toString().trimmed();
        if (!text.isEmpty()) {
            result.append(text);
        }
    }
    return result;
}
}

SolverPluginDescriptor OpenFoamPlugin::descriptor() const
{
    SolverPluginDescriptor descriptor;
    descriptor.id = "openfoam";
    descriptor.name = "OpenFOAM";
    descriptor.vendor = "OpenFOAM Foundation / OpenCFD";
    descriptor.version = "demo-service";
    descriptor.solverFamily = "cfd";
    descriptor.description = "FastAPI demo integration for OpenFOAM-style remote solver execution.";
    descriptor.status = SolverPluginStatus::Ready;
    descriptor.capabilities.canExportCase = true;
    descriptor.capabilities.canRunCase = true;
    descriptor.capabilities.canReadResult = true;
    descriptor.capabilities.analysisTypes = {"incompressibleFlow"};
    descriptor.capabilities.inputFormats = {"OpenFOAM service JSON"};
    descriptor.capabilities.outputFormats = {"VTK", "service log"};
    return descriptor;
}

SolverCaseWriterResult OpenFoamPlugin::exportCase(const SolverCaseContext &context) const
{
    const OpenFoamCaseWriter writer;
    return writer.write(context);
}

SolverRunResult OpenFoamPlugin::runCase(const SolverCaseContext &context) const
{
    SolverRunResult result;
    result.command = "POST " + OpenFoamServiceClient::defaultBaseUrl().toString(QUrl::StripTrailingSlash) + "/run";
    result.workingDirectory = context.caseDirectory;
    result.logFile = OpenFoamServiceClient::serviceLogFilePath(context.caseDirectory);

    const OpenFoamServiceClient client;
    const OpenFoamServiceRunResult serviceResult = client.runDemoCase(context);
    result.logMessages.append("OpenFOAM service command: " + result.command);
    result.logMessages.append("OpenFOAM service response: " + OpenFoamServiceClient::responseFilePath(context.caseDirectory));
    result.logMessages.append(serviceResult.logMessages);
    result.errors.append(serviceResult.errors);
    result.standardOutput = serviceResult.logMessages.join('\n');
    result.standardError = serviceResult.errors.join('\n');
    result.exitCode = serviceResult.success ? 0 : 1;
    result.success = serviceResult.success;
    return result;
}

SolverResultReadResult OpenFoamPlugin::readResult(const SolverCaseContext &context) const
{
    SolverResultReadResult result;
    const QString responsePath = OpenFoamServiceClient::responseFilePath(context.caseDirectory);
    QFile file(responsePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errors.append("OpenFOAM result read failed: cannot open service response: " + file.errorString());
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.errors.append("OpenFOAM result read failed: invalid service response JSON: " + parseError.errorString());
        return result;
    }

    const QJsonObject response = document.object();
    const QStringList vtkFiles = jsonStringArray(response.value("vtkFiles"));
    if (vtkFiles.isEmpty()) {
        result.errors.append("OpenFOAM result read failed: service response did not include VTK files.");
        return result;
    }

    for (const QString &vtkFile : vtkFiles) {
        if (!QFileInfo::exists(vtkFile)) {
            result.warnings.append("OpenFOAM VTK result file does not exist: " + vtkFile);
        }
    }

    result.success = true;
    result.summary = response.value("summary").toString("OpenFOAM demo result read: VTK files=" + QString::number(vtkFiles.size()));
    result.resultFiles.append(vtkFiles);
    result.resultFiles.append(responsePath);

    const QString logFile = response.value("logFile").toString();
    if (!logFile.isEmpty()) {
        result.resultFiles.append(logFile);
    }

    result.logMessages.append("OpenFOAM VTK result files: " + QString::number(vtkFiles.size()));
    for (const QString &vtkFile : vtkFiles) {
        result.logMessages.append("OpenFOAM VTK: " + vtkFile);
    }
    return result;
}
