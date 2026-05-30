#include "workflow/SolverCaseWorkflowController.h"

#include "project/ProjectModel.h"
#include "result/ResultHistoryNormalizer.h"
#include "result/ResultObject.h"
#include "result/ResultManager.h"
#include "mesh/MeshData.h"
#include "mesh/MeshObject.h"
#include "mesh/MeshQualityService.h"
#include "mesh/MshReader.h"
#include "solver/BoundaryBindingInspector.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/calculix/CalculiXEnvironment.h"
#include "solver/calculix/CalculiXSectionAssignmentValidator.h"
#include "solver/SimulationCase.h"
#include "solver/SimulationCaseBuilder.h"
#include "solver/export/SolverCaseWriter.h"
#include "solver/plugin/SolverPlugin.h"
#include "solver/plugin/SolverPluginManager.h"
#include "units/UnitConverter.h"
#include "workflow/ProjectWorkflowController.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

void appendMessages(SolverCaseWorkflowResult &result, const QStringList &messages)
{
    result.logMessages.append(messages);
}

void appendErrors(SolverCaseWorkflowResult &result, const QString &prefix, const QStringList &errors)
{
    for (const QString &error : errors) {
        result.logMessages.append(prefix + error);
    }
}

void appendWarnings(SolverCaseWorkflowResult &result, const QString &prefix, const QStringList &warnings)
{
    for (const QString &warning : warnings) {
        result.logMessages.append(prefix + warning);
    }
}

QString sanitizedDirectoryStem(QString value)
{
    value = value.trimmed();
    QString sanitized;
    sanitized.reserve(value.size());
    bool hasUsefulCharacter = false;

    for (const QChar ch : value) {
        if (ch.isLetterOrNumber()) {
            sanitized.append(ch);
            hasUsefulCharacter = true;
        } else if (ch.isSpace() || ch == '_' || ch == '-') {
            sanitized.append('_');
        }
    }

    return hasUsefulCharacter ? sanitized : QString();
}

QString simulationCaseStem(const SimulationCase &simulationCase)
{
    const QString nameStem = sanitizedDirectoryStem(simulationCase.name);
    if (!nameStem.isEmpty()) {
        return nameStem;
    }

    const QString idStem = sanitizedDirectoryStem(simulationCase.id);
    return idStem.isEmpty() ? QString("simulation_case") : idStem;
}

QString solverCaseDirectory(
    const ProjectModel &projectModel,
    const QString &pluginId,
    const SimulationCase &simulationCase
)
{
    const QString pluginStem = sanitizedDirectoryStem(pluginId);
    const QString safePluginId = pluginStem.isEmpty()
        ? QString("solver")
        : pluginStem;
    const QString runName = QString("%1_%2")
        .arg(simulationCaseStem(simulationCase))
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz"));
    return QDir(projectModel.project().rootPath).filePath("solver/" + safePluginId + "/" + runName);
}

QString simulationMeshName(const QString &pluginId, const SimulationCase &simulationCase)
{
    if (pluginId == "calculix" && !simulationCase.structuralCase.meshName.trimmed().isEmpty()) {
        return simulationCase.structuralCase.meshName;
    }
    if (pluginId == "openfoam" && !simulationCase.cfdCase.meshName.trimmed().isEmpty()) {
        return simulationCase.cfdCase.meshName;
    }
    return simulationCase.meshName;
}

struct SolverPreflightResult
{
    bool passed = true;
    QStringList messages;
};

void addPreflightError(SolverPreflightResult &result, const QString &message)
{
    result.passed = false;
    result.messages.append("Preflight error: " + message);
}

void addPreflightWarning(SolverPreflightResult &result, const QString &message)
{
    result.messages.append("Preflight warning: " + message);
}

bool isStructuralConstraintBoundary(const BoundaryCondition &boundaryCondition, const std::vector<Load> &loads)
{
    if (!boundaryCondition.enabled) {
        return false;
    }
    if (boundaryCondition.type == BoundaryConditionType::FixedSupport
            || boundaryCondition.type == BoundaryConditionType::Displacement) {
        return true;
    }
    return boundaryCondition.type == BoundaryConditionType::Wall
        && !hasStructuralLoadForBoundary(loads, boundaryCondition.id);
}

bool loadReferencesExistingBoundary(const Load &load, const std::vector<BoundaryCondition> &boundaries)
{
    if (load.type == LoadType::Gravity) {
        return true;
    }
    for (const BoundaryCondition &boundary : boundaries) {
        if (boundary.id == load.boundaryConditionId && boundary.enabled) {
            return true;
        }
    }
    return false;
}

bool vectorIsZero(const LoadValue &value)
{
    return value.x == 0.0 && value.y == 0.0 && value.z == 0.0;
}

QString normalizedPropertyName(QString name)
{
    name = name.trimmed().toLower();
    QString normalized;
    normalized.reserve(name.size());
    for (const QChar ch : name) {
        if (ch.isLetterOrNumber()) {
            normalized.append(ch);
        }
    }
    return normalized;
}

bool isYoungModulusProperty(const QString &propertyName)
{
    const QString normalized = normalizedPropertyName(propertyName);
    return normalized == "youngmodulus"
        || normalized == "youngsmodulus"
        || normalized == "elasticmodulus"
        || normalized == "e";
}

bool isDensityProperty(const QString &propertyName)
{
    const QString normalized = normalizedPropertyName(propertyName);
    return normalized == "density" || normalized == "rho";
}

void validateStructuralMaterialUnits(SolverPreflightResult &preflight, const std::vector<Material> &materials)
{
    for (const Material &material : materials) {
        if (material.hasDensity && !UnitConverter::isKnownUnit(UnitQuantity::Density, material.densityUnit)) {
            addPreflightError(preflight, "material density has unknown unit: "
                + material.name + ", unit=" + material.densityUnit + ".");
        }
        for (const MaterialProperty &property : material.extraProperties) {
            if (isYoungModulusProperty(property.name)
                    && !UnitConverter::isKnownUnit(UnitQuantity::Stress, property.unit)) {
                addPreflightError(preflight, "material elastic modulus has unknown unit: "
                    + material.name + ", property=" + property.name + ", unit=" + property.unit + ".");
            } else if (isDensityProperty(property.name)
                    && !UnitConverter::isKnownUnit(UnitQuantity::Density, property.unit)) {
                addPreflightError(preflight, "material density property has unknown unit: "
                    + material.name + ", property=" + property.name + ", unit=" + property.unit + ".");
            }
        }
    }
}

void validateStructuralLoad(SolverPreflightResult &preflight, const Load &load)
{
    if (load.type == LoadType::BodyForce || load.type == LoadType::Temperature) {
        addPreflightError(preflight, "CalculiX load type is not supported yet: "
            + load.name + " (" + toString(load.type) + ").");
        return;
    }

    if (load.type == LoadType::Pressure) {
        if (load.value.kind != LoadValueKind::Scalar) {
            addPreflightError(preflight, "pressure load must use a scalar value: " + load.name + ".");
        }
        if (!UnitConverter::isKnownUnit(UnitQuantity::Stress, load.value.unit)) {
            addPreflightError(preflight, "pressure load has unknown unit: "
                + load.name + ", unit=" + load.value.unit + ".");
        }
    } else if (load.type == LoadType::Force || load.type == LoadType::SurfaceForce) {
        if (load.value.kind == LoadValueKind::Vector3 && vectorIsZero(load.value)) {
            addPreflightWarning(preflight, "force load vector is zero: " + load.name + ".");
        }
        if (!UnitConverter::isKnownUnit(UnitQuantity::Force, load.value.unit)) {
            addPreflightError(preflight, "force load has unknown unit: "
                + load.name + ", unit=" + load.value.unit + ".");
        }
        if (load.type == LoadType::SurfaceForce) {
            addPreflightWarning(preflight, "SurfaceForce 当前按目标边界节点平均分配为 *CLOAD；不是真实 surface traction: "
                + load.name + ".");
        }
    } else if (load.type == LoadType::Gravity) {
        if (load.value.kind != LoadValueKind::Vector3 || vectorIsZero(load.value)) {
            addPreflightError(preflight, "gravity load must use a non-zero vector value: " + load.name + ".");
        }
        if (!UnitConverter::isKnownUnit(UnitQuantity::Acceleration, load.value.unit)) {
            addPreflightError(preflight, "gravity load has unknown unit: "
                + load.name + ", unit=" + load.value.unit + ".");
        }
    }
}

void validateMeshPreflight(
    SolverPreflightResult &preflight,
    const ProjectModel &projectModel,
    const QString &meshName
)
{
    if (meshName.trimmed().isEmpty()) {
        addPreflightError(preflight, "no mesh is selected for this simulation case. 请先生成或导入网格。");
        return;
    }

    const MeshObject *meshObject = projectModel.findMeshByName(meshName);
    if (!meshObject) {
        addPreflightError(preflight, "mesh not found: " + meshName + ". 请重新选择或导入网格。");
        return;
    }
    if (meshObject->mshFile.trimmed().isEmpty()) {
        addPreflightError(preflight, "mesh object has no .msh file path: " + meshObject->name + ". 请重新生成或导入网格。");
        return;
    }

    const QString meshPath = QFileInfo(meshObject->mshFile).isAbsolute()
        ? meshObject->mshFile
        : QDir(projectModel.project().rootPath).filePath(meshObject->mshFile);
    if (!QFileInfo::exists(meshPath)) {
        addPreflightError(preflight, "mesh file does not exist: " + meshPath + ". 请重新生成网格。");
    }
    if (meshObject->stale) {
        addPreflightError(preflight, "mesh is stale. 网格已过期，请重新生成网格后再求解。");
        if (!meshObject->staleReason.isEmpty()) {
            addPreflightWarning(preflight, "mesh stale reason: " + meshObject->staleReason);
        }
    }

    preflight.messages.append(MeshQualityService::solverPreflightMessages(*meshObject));
    if (MeshQualityService::hasCriticalIssues(*meshObject)) {
        addPreflightError(preflight, "mesh quality has critical issues. 请修复退化单元、负体积或无效单元。");
    }
}

QString absoluteProjectPath(const ProjectModel &projectModel, const QString &path)
{
    return QFileInfo(path).isAbsolute()
        ? path
        : QDir(projectModel.project().rootPath).filePath(path);
}

bool readMeshDataForPreflight(
    SolverPreflightResult &preflight,
    const ProjectModel &projectModel,
    const MeshObject &meshObject,
    MeshData &meshData
)
{
    const QString meshPath = absoluteProjectPath(projectModel, meshObject.mshFile);
    if (meshObject.mshFile.trimmed().isEmpty() || !QFileInfo::exists(meshPath)) {
        addPreflightError(preflight, "cannot read MSH for CalculiX section assignment validation: mesh file is missing: "
            + meshPath + ". 请重新生成或导入网格。");
        return false;
    }

    QString meshReadError;
    meshData = MeshData{};
    meshData.name = meshObject.name;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    meshData.mshFilePath = meshPath;
    if (!MshReader::readMsh2(meshPath, meshData, &meshReadError)) {
        addPreflightError(preflight, "cannot read MSH for CalculiX section assignment validation: "
            + meshReadError + ". 请重新生成或导入网格。");
        return false;
    }
    meshData.name = meshObject.name;
    meshData.sourceGeometryName = meshObject.sourceGeometryName;
    meshData.mshFilePath = meshPath;
    if (meshData.nodes.empty() || meshData.tetraCount() == 0) {
        addPreflightError(preflight, "mesh has no tetrahedral elements for CalculiX section assignment validation. 请重新生成三维实体网格。");
        return false;
    }
    return true;
}

void validateCalculiXSectionAssignments(
    SolverPreflightResult &preflight,
    const StructuralCase &structuralCase,
    const MeshObject &meshObject,
    const MeshData &meshData
)
{
    const CalculiXSectionValidationResult sectionValidation =
        CalculiXSectionAssignmentValidator::validate(structuralCase, meshObject, meshData);
    for (const QString &error : sectionValidation.errors) {
        addPreflightError(preflight, error);
    }
    for (const QString &warning : sectionValidation.warnings) {
        addPreflightWarning(preflight, warning);
    }
}

void validateBoundaryTargets(
    SolverPreflightResult &preflight,
    const ProjectModel &projectModel,
    const QString &meshName,
    const std::vector<BoundaryCondition> &boundaries,
    const std::vector<FaceGroup> &faceGroups,
    const std::vector<Load> &loads
)
{
    for (const BoundaryCondition &boundaryCondition : boundaries) {
        if (!boundaryCondition.enabled) {
            continue;
        }
        if (boundaryCondition.target.kind == BoundaryTargetKind::MeshBoundary) {
            bool found = false;
            for (const MeshBoundary &meshBoundary : projectModel.meshRepository().meshBoundaries()) {
                if (meshBoundary.meshName != meshName) {
                    continue;
                }
                if (meshBoundary.id == boundaryCondition.target.meshBoundaryName
                        || meshBoundary.name == boundaryCondition.target.meshBoundaryName
                        || meshBoundary.physicalGroupName == boundaryCondition.target.meshBoundaryName) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                addPreflightError(preflight, "mesh boundary target is missing: "
                    + boundaryCondition.name + ". 网格边界丢失，请重新生成网格或重新绑定边界。");
            }
            continue;
        }

        const BoundaryConditionBindingSummary summary =
            BoundaryBindingInspector::summarizeBoundaryCondition(boundaryCondition, faceGroups, loads);
        if (!summary.faceGroupExists) {
            addPreflightError(preflight, "boundary target face group is missing: "
                + boundaryCondition.name + ". 面组丢失，请重新绑定边界。");
        } else if (summary.faceGroupIsEmpty) {
            addPreflightError(preflight, "boundary target face group is empty: "
                + boundaryCondition.name + ". 目标面组为空，请重新拾取面。");
        }
        for (const QString &warning : summary.warnings) {
            addPreflightWarning(preflight, "boundary binding warning for "
                + boundaryCondition.name + ": " + warning);
        }
    }
}

SolverPreflightResult validateCalculiXPreflight(
    const ProjectModel &projectModel,
    const SimulationCase &simulationCase,
    const QString &meshName
)
{
    SolverPreflightResult preflight;
    if (projectModel.geometryObjects().isEmpty()) {
        addPreflightError(preflight, "project has no geometry. 请先创建或导入几何。");
    }

    validateMeshPreflight(preflight, projectModel, meshName);

    const StructuralCase &structuralCase = simulationCase.structuralCase;
    validateStructuralMaterialUnits(preflight, structuralCase.materials);
    for (const BoundaryCondition &boundaryCondition : simulationCase.boundaryConditions) {
        if (boundaryCondition.enabled && boundaryCondition.type == BoundaryConditionType::SymmetryStructural) {
            addPreflightError(preflight, "structural symmetry boundary is not supported by CalculiX export yet: "
                + boundaryCondition.name + ". Please use FixedSupport or Displacement constraints.");
        }
    }
    if (const MeshObject *meshObject = projectModel.findMeshByName(meshName)) {
        MeshData meshData;
        if (readMeshDataForPreflight(preflight, projectModel, *meshObject, meshData)) {
            validateCalculiXSectionAssignments(preflight, structuralCase, *meshObject, meshData);
        }
    }

    bool hasConstraint = false;
    for (const BoundaryCondition &boundaryCondition : structuralCase.constraints) {
        hasConstraint = hasConstraint || isStructuralConstraintBoundary(boundaryCondition, structuralCase.loads);
    }
    if (!hasConstraint) {
        addPreflightError(preflight, "missing fixed/displacement constraint; the model may move as a rigid body. 缺少固定约束，模型可能刚体运动。");
    }
    if (structuralCase.loads.empty()) {
        addPreflightError(preflight, "missing structural load. 请添加压力、力或重力载荷。");
    }
    for (const Load &load : structuralCase.loads) {
        if (!load.enabled) {
            continue;
        }
        if (!loadReferencesExistingBoundary(load, structuralCase.constraints)) {
            addPreflightError(preflight, "load target boundary is missing for load: "
                + load.name + ". 载荷引用的边界已丢失，请重新选择载荷作用面。");
        }
        validateStructuralLoad(preflight, load);
    }

    validateBoundaryTargets(
        preflight,
        projectModel,
        meshName,
        structuralCase.constraints,
        simulationCase.geometrySetup.faceGroups,
        structuralCase.loads
    );

    QString resolvedCcx;
    if (!CalculiXEnvironment::executableAvailable(&resolvedCcx)) {
        addPreflightError(preflight, "ccx executable is not available: "
            + CalculiXEnvironment::executablePath()
            + ". 请配置 MYCAE_CALCULIX_EXECUTABLE 或 CMake 中的 CalculiX 路径。");
    } else {
        preflight.messages.append("Preflight info: CalculiX executable found: " + resolvedCcx);
    }

    if (preflight.passed) {
        preflight.messages.prepend("Preflight info: CalculiX preflight checks passed.");
    }
    return preflight;
}

ResultObject makeResultObject(
    const QString &pluginId,
    const SolverPluginDescriptor &descriptor,
    const SimulationCase &simulationCase,
    const QString &caseDirectory,
    const SolverRunResult &runResult,
    const SolverResultReadResult &readResult
)
{
    ResultObject resultObject;
    const QString createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    resultObject.id = ResultHistoryNormalizer::makeResultId(pluginId);
    resultObject.name = ResultHistoryNormalizer::makeResultName(
        descriptor.name,
        simulationCase.name,
        caseDirectory,
        createdAt
    );
    resultObject.solverName = descriptor.name;
    resultObject.meshName = descriptor.id == "openfoam" && !simulationCase.cfdCase.meshName.trimmed().isEmpty()
        ? simulationCase.cfdCase.meshName
        : (descriptor.id == "calculix" && !simulationCase.structuralCase.meshName.trimmed().isEmpty()
            ? simulationCase.structuralCase.meshName
            : simulationCase.meshName);
    resultObject.casePath = caseDirectory;
    resultObject.logFile = runResult.logFile;
    resultObject.resultFiles = readResult.resultFiles;
    for (const QString &filePath : readResult.resultFiles) {
        const QString suffix = QFileInfo(filePath).suffix().toLower();
        if (suffix == "dat") {
            resultObject.datFile = filePath;
        } else if (suffix == "frd") {
            resultObject.frdFile = filePath;
        } else if (suffix == "sta") {
            resultObject.staFile = filePath;
        }
    }
    if (descriptor.id == "calculix") {
        resultObject.primaryFieldName = CalculiXResultFields::DisplacementMagnitude;
        resultObject.displayFieldName = resultObject.primaryFieldName;
        resultObject.deformationScale = 0.0;
        resultObject.availableFields = QStringList{
            CalculiXResultFields::Ux,
            CalculiXResultFields::Uy,
            CalculiXResultFields::Uz,
            CalculiXResultFields::DisplacementMagnitude,
            CalculiXResultFields::VonMisesStress
        };
    }
    resultObject.createdAt = createdAt;
    resultObject.success = true;
    resultObject.summary = readResult.summary;
    return resultObject;
}
}

SolverCaseWorkflowController::SolverCaseWorkflowController(
    ProjectModel &projectModel,
    const SolverPluginManager &solverPluginManager,
    ProjectWorkflowController &projectWorkflow
)
    : m_projectModel(projectModel)
    , m_solverPluginManager(solverPluginManager)
    , m_projectWorkflow(projectWorkflow)
{
}

SolverCaseWorkflowResult SolverCaseWorkflowController::runPlugin(const QString &pluginId) const
{
    SolverCaseWorkflowResult result;
    if (!m_projectModel.hasProject()) {
        result.logMessages.append(zh(u8"运行求解器失败：请先创建或打开工程。"));
        return result;
    }

    const ProjectWorkflowResult saveResult = m_projectWorkflow.saveSimulationCase();
    result.logMessages.append(saveResult.logMessages);
    if (!saveResult.success) {
        return result;
    }

    const SolverPlugin *plugin = m_solverPluginManager.pluginById(pluginId);
    if (!plugin) {
        result.logMessages.append(zh(u8"运行求解器失败：插件未注册：") + pluginId);
        return result;
    }

    const SolverPluginDescriptor *descriptor = m_solverPluginManager.descriptorById(pluginId);
    if (!descriptor) {
        result.logMessages.append(zh(u8"运行求解器失败：插件描述未注册：") + pluginId);
        return result;
    }

    const SimulationCase simulationCase = SimulationCaseBuilder::fromProjectModel(m_projectModel);
    const QString meshName = simulationMeshName(pluginId, simulationCase);
    if (pluginId == "calculix") {
        const SolverPreflightResult preflight =
            validateCalculiXPreflight(m_projectModel, simulationCase, meshName);
        result.logMessages.append(preflight.messages);
        if (!preflight.passed) {
            return result;
        }
    }
    if (const MeshObject *meshObject = m_projectModel.findMeshByName(meshName)) {
        if (meshObject->stale) {
            result.logMessages.append(zh(u8"运行求解器失败：网格已过期，请重新生成网格后再求解。"));
            if (!meshObject->staleReason.isEmpty()) {
                result.logMessages.append(zh(u8"网格过期原因：") + meshObject->staleReason);
            }
            return result;
        }
        result.logMessages.append(MeshQualityService::solverPreflightMessages(*meshObject));
        if (MeshQualityService::hasCriticalIssues(*meshObject)) {
            return result;
        }
    }
    const QString caseDirectory = solverCaseDirectory(m_projectModel, pluginId, simulationCase);
    if (!QDir().mkpath(caseDirectory)) {
        result.logMessages.append(zh(u8"运行求解器失败：无法创建算例目录：") + caseDirectory);
        return result;
    }

    result.logMessages.append(zh(u8"求解器插件：") + descriptor->name + " (" + descriptor->id + ")");
    result.logMessages.append(zh(u8"求解器类型：") + descriptor->solverFamily);
    result.logMessages.append(zh(u8"求解算例目录：") + caseDirectory);
    if (pluginId == "calculix") {
        result.logMessages.append(QString("Structural case data: %1 material(s), %2 constraint/load target(s), %3 load(s).")
            .arg(simulationCase.structuralCase.materials.size())
            .arg(simulationCase.structuralCase.constraints.size())
            .arg(simulationCase.structuralCase.loads.size()));
    } else if (pluginId == "openfoam") {
        result.logMessages.append(QString("CFD case data: %1 fluid material(s), %2 boundary condition(s), %3 field value(s).")
            .arg(simulationCase.cfdCase.materials.size())
            .arg(simulationCase.cfdCase.boundaries.size())
            .arg(simulationCase.cfdCase.fieldValues.size()));
    } else {
        result.logMessages.append(zh(u8"求解算例数据：%1 个材料，%2 个边界条件，%3 个载荷。")
            .arg(simulationCase.materials.size())
            .arg(simulationCase.boundaryConditions.size())
            .arg(simulationCase.loads.size()));
    }

    if (!descriptor->isUsable()) {
        result.logMessages.append(zh(u8"运行求解器失败：插件为预留状态或不可用。"));
        return result;
    }

    const SolverCaseContext context{&m_projectModel, &simulationCase, caseDirectory, pluginId};

    const SolverCaseWriter solverCaseWriter;
    const SolverCaseWriterResult exportResult = solverCaseWriter.writeCase(*plugin, context);
    appendMessages(result, exportResult.logMessages);
    appendWarnings(result, zh(u8"求解器导出警告："), exportResult.warnings);
    appendErrors(result, zh(u8"求解器导出失败："), exportResult.errors);
    if (!exportResult.success) {
        return result;
    }
    result.logMessages.append(zh(u8"求解器输入已导出。"));

    const SolverRunResult runResult = plugin->runCase(context);
    appendMessages(result, runResult.logMessages);
    appendErrors(result, zh(u8"求解器运行失败："), runResult.errors);
    if (!runResult.success) {
        return result;
    }

    const SolverResultReadResult readResult = plugin->readResult(context);
    appendMessages(result, readResult.logMessages);
    appendWarnings(result, zh(u8"求解结果警告："), readResult.warnings);
    appendErrors(result, zh(u8"求解结果读取失败："), readResult.errors);
    if (!readResult.success) {
        return result;
    }

    ResultObject resultObject = makeResultObject(pluginId, *descriptor, simulationCase, caseDirectory, runResult, readResult);
    m_projectModel.resultRepository().results().push_back(resultObject);
    QStringList normalizeMessages;
    ResultHistoryNormalizer::normalize(m_projectModel.resultRepository().results(), &normalizeMessages);
    appendMessages(result, normalizeMessages);
    const ResultObject &savedResultObject = m_projectModel.resultRepository().results().back();
    result.resultId = savedResultObject.id;
    result.logMessages.append(zh(u8"结果记录已创建：") + savedResultObject.name + " (" + savedResultObject.id + ")");
    QString saveError;
    if (!ResultManager().save(m_projectModel.project(), m_projectModel.resultRepository().results(), &saveError)) {
        result.logMessages.append(zh(u8"保存结果索引失败：") + saveError);
    } else {
        result.logMessages.append(zh(u8"结果索引已保存：") + ResultManager::relativeResultsFilePath());
    }

    result.logMessages.append(zh(u8"求解结果：") + readResult.summary);
    result.success = true;
    return result;
}
