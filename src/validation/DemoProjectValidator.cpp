#include "validation/DemoProjectValidator.h"

#include "geometry/GeometryManager.h"
#include "project/ProjectManager.h"
#include "project/ProjectModel.h"
#include "project/ProjectModelLoader.h"
#include "project/ProjectResourceManager.h"
#include "project/ResultRepository.h"
#include "result/ResultCsvExporter.h"
#include "result/ResultDataLoader.h"
#include "result/ResultObject.h"
#include "result/ResultReportExporter.h"
#include "validation/SampleProjectValidator.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <memory>
#include <utility>

namespace
{
constexpr auto DemoProjectRelativePath = "projects/box_pressure_demo/project.json";
constexpr auto DemoStepPrefix = "box pressure demo ";

void addStep(SampleValidationReport &report, bool passed, const QString &name, const QString &detail = {})
{
    report.addStep(passed ? SampleValidationStatus::Pass : SampleValidationStatus::Fail, name, detail);
}

void validateCount(SampleValidationReport &report, const QString &name, qsizetype actual, qsizetype expected)
{
    addStep(report, actual == expected, name, QString("count=%1").arg(actual));
}

QStringList resourceProblems(const ProjectResourceReport &report)
{
    QStringList problems;
    for (const ProjectResourceItem &item : report.items) {
        if (item.status != ProjectResourceStatus::Valid) {
            problems.append(QString("%1/%2=%3")
                .arg(item.category, item.name, projectResourceStatusName(item.status)));
        }
    }
    return problems;
}

bool loadProjectData(ProjectModel &projectModel, QString *errorMessage)
{
    const GeometryManager geometryManager;
    const ProjectModelLoader loader(geometryManager);
    if (!loader.loadGeometries(projectModel, errorMessage)) {
        return false;
    }
    if (!loader.loadMeshes(projectModel, errorMessage)) {
        return false;
    }
    if (!loader.loadSimulationCase(projectModel, errorMessage)) {
        return false;
    }
    if (!loader.loadResults(projectModel, errorMessage)) {
        return false;
    }
    return true;
}

void validateProjectModel(const ProjectModel &projectModel, SampleValidationReport &report)
{
    validateCount(report, DemoStepPrefix + QString("geometry count"), projectModel.geometryObjects().size(), 1);
    validateCount(report, DemoStepPrefix + QString("mesh count"), projectModel.meshObjects().size(), 1);
    validateCount(report, DemoStepPrefix + QString("material count"), projectModel.materials().size(), 1);
    validateCount(report, DemoStepPrefix + QString("boundary condition count"), projectModel.boundaryConditions().size(), 2);
    validateCount(report, DemoStepPrefix + QString("load count"), projectModel.loads().size(), 1);
    validateCount(report, DemoStepPrefix + QString("result count"), projectModel.resultRepository().results().size(), 1);
}

void validateResources(const ProjectModel &projectModel, SampleValidationReport &report)
{
    const ProjectResourceReport resourceReport = ProjectResourceManager().inspect(projectModel);
    const QStringList problems = resourceProblems(resourceReport);
    addStep(
        report,
        problems.isEmpty(),
        DemoStepPrefix + QString("resources complete"),
        problems.isEmpty() ? projectResourceSizeText(resourceReport.totalBytes) : problems.join("; ")
    );
}

void validateResultData(const ProjectModel &projectModel, SampleValidationReport &report)
{
    if (projectModel.resultRepository().results().empty()) {
        return;
    }

    const ResultObject &resultObject = projectModel.resultRepository().results().front();
    auto loadedResult = std::make_unique<ResultDataLoadResult>(
        ResultDataLoader().loadCalculiXResult(projectModel, resultObject)
    );
    if (!loadedResult->success) {
        report.addStep(
            SampleValidationStatus::Fail,
            DemoStepPrefix + QString("result data readable"),
            loadedResult->errors.join("; ")
        );
        return;
    }

    report.addStep(
        SampleValidationStatus::Pass,
        DemoStepPrefix + QString("result data readable"),
        QString("nodes=%1; stresses=%2")
            .arg(loadedResult->datResult.displacements.size())
            .arg(loadedResult->datResult.stresses.size())
    );
    loadedResult.release();
}

QString demoExportRoot()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .filePath("MyCAE/sample_validation/demo_exports");
}

void validateResultExports(const ProjectModel &projectModel, SampleValidationReport &report)
{
    if (projectModel.resultRepository().results().empty()) {
        return;
    }

    const ResultObject &resultObject = projectModel.resultRepository().results().front();
    const QString exportRoot = demoExportRoot();
    QDir(exportRoot).removeRecursively();
    if (!QDir().mkpath(exportRoot)) {
        report.addStep(SampleValidationStatus::Fail, DemoStepPrefix + QString("export directory created"), exportRoot);
        return;
    }

    const QString csvPath = QDir(exportRoot).filePath("box_pressure_result.csv");
    const ResultCsvExportResult csvResult = ResultCsvExporter().exportResult(projectModel, resultObject, csvPath);
    addStep(
        report,
        csvResult.success
            && QFileInfo::exists(csvResult.nodeDisplacementCsvPath)
            && QFileInfo::exists(csvResult.elementStressCsvPath)
            && QFileInfo::exists(csvResult.summaryCsvPath)
            && QFileInfo::exists(csvResult.metadataJsonPath),
        DemoStepPrefix + QString("result CSV export works"),
        csvResult.success ? exportRoot : csvResult.errorMessage
    );

    const QString reportPath = QDir(exportRoot).filePath("box_pressure_report.md");
    const ResultReportExportResult reportResult =
        ResultReportExporter().exportMarkdown(projectModel.project(), resultObject, reportPath, QString());
    addStep(
        report,
        reportResult.success && QFileInfo::exists(reportResult.reportPath),
        DemoStepPrefix + QString("result report export works"),
        reportResult.success ? reportResult.reportPath : reportResult.errorMessage
    );
}
}

DemoProjectValidator::DemoProjectValidator(QString samplesRoot)
    : m_samplesRoot(std::move(samplesRoot))
{
}

void DemoProjectValidator::validate(SampleValidationReport &report) const
{
    const QString projectFilePath = QDir(m_samplesRoot).filePath(DemoProjectRelativePath);
    if (!QFileInfo::exists(projectFilePath)) {
        report.addStep(SampleValidationStatus::Fail, DemoStepPrefix + QString("project found"), projectFilePath);
        return;
    }
    report.addStep(SampleValidationStatus::Pass, DemoStepPrefix + QString("project found"), projectFilePath);

    Project project;
    QString errorMessage;
    if (!ProjectManager().openProject(projectFilePath, &project, &errorMessage)) {
        report.addStep(SampleValidationStatus::Fail, DemoStepPrefix + QString("project opened"), errorMessage);
        return;
    }
    report.addStep(SampleValidationStatus::Pass, DemoStepPrefix + QString("project opened"), project.rootPath);

    ProjectModel projectModel;
    projectModel.setProject(project);
    if (!loadProjectData(projectModel, &errorMessage)) {
        report.addStep(SampleValidationStatus::Fail, DemoStepPrefix + QString("project data loaded"), errorMessage);
        return;
    }

    validateProjectModel(projectModel, report);
    validateResources(projectModel, report);
    validateResultData(projectModel, report);
    validateResultExports(projectModel, report);
}
