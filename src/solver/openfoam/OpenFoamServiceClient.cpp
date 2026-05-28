#include "solver/openfoam/OpenFoamServiceClient.h"

#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "solver/SimulationCase.h"

#include <QByteArray>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QtGlobal>

#include <utility>

namespace
{
constexpr int RequestTimeoutMs = 30000;

QString envServiceUrl()
{
    const QByteArray value = qgetenv("MYCAE_OPENFOAM_SERVICE_URL");
    return QString::fromUtf8(value).trimmed();
}

QUrl endpointUrl(QUrl baseUrl, const QString &path)
{
    QString text = baseUrl.toString(QUrl::StripTrailingSlash);
    if (!path.startsWith('/')) {
        text.append('/');
    }
    text.append(path);
    return QUrl(text);
}

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

bool writeJsonFile(const QString &filePath, const QJsonObject &object, QStringList &errors)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        errors.append("Cannot write OpenFOAM service response: " + file.errorString());
        return false;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

QString projectName(const SolverCaseContext &context)
{
    if (!context.projectModel || !context.projectModel->hasProject()) {
        return QString();
    }
    return context.projectModel->project().name;
}

const MeshObject *caseMesh(const SolverCaseContext &context)
{
    if (!context.projectModel || !context.simulationCase) {
        return nullptr;
    }
    const QString meshName = context.simulationCase->cfdCase.meshName.trimmed().isEmpty()
        ? context.simulationCase->meshName
        : context.simulationCase->cfdCase.meshName;
    return context.projectModel->findMeshByName(meshName);
}
}

OpenFoamServiceClient::OpenFoamServiceClient(QUrl baseUrl)
    : m_baseUrl(std::move(baseUrl))
{
}

QUrl OpenFoamServiceClient::defaultBaseUrl()
{
    const QString fromEnv = envServiceUrl();
    return QUrl(fromEnv.isEmpty() ? QStringLiteral("http://127.0.0.1:8765") : fromEnv);
}

QString OpenFoamServiceClient::responseFilePath(const QString &caseDirectory)
{
    return QDir(caseDirectory).filePath("openfoam_service_result.json");
}

QString OpenFoamServiceClient::serviceLogFilePath(const QString &caseDirectory)
{
    return QDir(caseDirectory).filePath("openfoam_demo.log");
}

QString OpenFoamServiceClient::requestManifestPath(const QString &caseDirectory)
{
    return QDir(caseDirectory).filePath("openfoam_case_request.json");
}

QString OpenFoamServiceClient::startCommandHint()
{
    return "python -m uvicorn openfoam_demo_service:app --host 127.0.0.1 --port 8765";
}

OpenFoamServiceRunResult OpenFoamServiceClient::runDemoCase(const SolverCaseContext &context) const
{
    OpenFoamServiceRunResult result;
    result.serviceUrl = m_baseUrl.toString(QUrl::StripTrailingSlash);

    if (!context.isValid()) {
        result.errors.append("OpenFOAM service request failed: invalid solver case context.");
        return result;
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(endpointUrl(m_baseUrl, "/run"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QJsonObject payload = buildRequest(context);
    QNetworkReply *reply = manager.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(RequestTimeoutMs);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        result.errors.append("OpenFOAM service request timed out after 30 seconds.");
        result.errors.append("Start the demo service first: " + startCommandHint());
        reply->deleteLater();
        return result;
    }

    result.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString networkErrorText = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError) {
        result.errors.append("OpenFOAM service request failed: " + networkErrorText);
        result.errors.append("Service URL: " + result.serviceUrl);
        result.errors.append("Start the demo service first: " + startCommandHint());
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.errors.append("OpenFOAM service returned invalid JSON: " + parseError.errorString());
        return result;
    }

    result.response = document.object();
    result.success = result.response.value("success").toBool(false);
    result.jobId = result.response.value("jobId").toString();
    result.summary = result.response.value("summary").toString();
    result.logFile = result.response.value("logFile").toString(serviceLogFilePath(context.caseDirectory));
    result.logMessages = jsonStringArray(result.response.value("log"));
    result.vtkFiles = jsonStringArray(result.response.value("vtkFiles"));

    if (!writeJsonFile(responseFilePath(context.caseDirectory), result.response, result.errors)) {
        result.success = false;
    }
    if (!result.success && result.errors.isEmpty()) {
        result.errors.append("OpenFOAM service reported failure.");
    }

    return result;
}

QJsonObject OpenFoamServiceClient::buildRequest(const SolverCaseContext &context) const
{
    const MeshObject *mesh = caseMesh(context);
    const SimulationCase *simulationCase = context.simulationCase;
    const CfdCase *cfdCase = simulationCase ? &simulationCase->cfdCase : nullptr;

    QJsonObject metadata;
    metadata.insert("solver", "openfoam-demo");
    metadata.insert("serviceUrl", m_baseUrl.toString(QUrl::StripTrailingSlash));

    QJsonObject payload;
    payload.insert("caseName", context.caseName);
    payload.insert("caseDirectory", QDir::toNativeSeparators(context.caseDirectory));
    payload.insert("projectName", projectName(context));
    payload.insert("meshName", cfdCase && !cfdCase->meshName.trimmed().isEmpty()
        ? cfdCase->meshName
        : (simulationCase ? simulationCase->meshName : QString()));
    payload.insert("geometryCount", context.projectModel ? static_cast<int>(context.projectModel->geometryObjects().size()) : 0);
    payload.insert("meshNodeCount", mesh ? mesh->nodeCount : 0);
    payload.insert("meshCellCount", mesh ? mesh->tetraCount : 0);
    payload.insert("materialCount", cfdCase ? static_cast<int>(cfdCase->materials.size()) : 0);
    payload.insert(
        "boundaryConditionCount",
        cfdCase ? static_cast<int>(cfdCase->boundaries.size()) : 0
    );
    payload.insert("loadCount", cfdCase ? static_cast<int>(cfdCase->fieldValues.size()) : 0);
    payload.insert("metadata", metadata);
    return payload;
}
