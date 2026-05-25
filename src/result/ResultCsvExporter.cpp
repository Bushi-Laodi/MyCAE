#include "result/ResultCsvExporter.h"

#include "mesh/MeshData.h"
#include "result/ResultDataLoader.h"
#include "result/ResultObject.h"
#include "solver/calculix/CalculiXResultMath.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <unordered_map>
#include <unordered_set>

namespace
{
struct StressAccumulator
{
    double vonMisesSum = 0.0;
    int count = 0;
};

QString elementStressCsvPathFor(const QString &nodeDisplacementCsvPath)
{
    const QFileInfo fileInfo(nodeDisplacementCsvPath);
    QString stem = fileInfo.completeBaseName();
    if (stem.endsWith("_result")) {
        stem.chop(QString("_result").size());
    } else if (stem.endsWith("_node_displacement")) {
        stem.chop(QString("_node_displacement").size());
    }
    return fileInfo.dir().filePath(stem + "_element_stress.csv");
}

bool ensureParentDirectory(const QString &filePath, QString *errorMessage)
{
    const QString parentPath = QFileInfo(filePath).absolutePath();
    if (QDir().mkpath(parentPath)) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = "Create CSV export directory failed: " + parentPath;
    }
    return false;
}

bool writeNodeDisplacementCsv(
    const MeshData &meshData,
    const CalculiXDatResult &datResult,
    const QString &filePath,
    QString *errorMessage
)
{
    std::unordered_map<int, CalculiXNodeDisplacement> displacementByNodeId;
    displacementByNodeId.reserve(datResult.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : datResult.displacements) {
        displacementByNodeId.insert_or_assign(displacement.nodeId, displacement);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Open node displacement CSV failed: " + file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setRealNumberPrecision(12);
    stream << "nodeId,x,y,z,Ux,Uy,Uz,UMag\n";

    for (const MeshNode &node : meshData.nodes) {
        CalculiXNodeDisplacement displacement;
        const auto displacementIt = displacementByNodeId.find(node.id);
        if (displacementIt != displacementByNodeId.end()) {
            displacement = displacementIt->second;
        }

        stream << node.id << ','
            << node.x << ','
            << node.y << ','
            << node.z << ','
            << displacement.ux << ','
            << displacement.uy << ','
            << displacement.uz << ','
            << CalculiXResultMath::displacementMagnitude(displacement) << '\n';
    }

    return true;
}

QStringList coverageWarnings(const MeshData &meshData, const CalculiXDatResult &datResult)
{
    QStringList warnings;

    std::unordered_set<int> displacementNodeIds;
    displacementNodeIds.reserve(datResult.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : datResult.displacements) {
        displacementNodeIds.insert(displacement.nodeId);
    }

    int matchedNodeCount = 0;
    for (const MeshNode &node : meshData.nodes) {
        if (displacementNodeIds.find(node.id) != displacementNodeIds.end()) {
            ++matchedNodeCount;
        }
    }
    if (matchedNodeCount < meshData.nodeCount()) {
        warnings.append(QString("CSV export warning: only %1 of %2 mesh nodes have displacement values; missing nodes use 0.")
            .arg(matchedNodeCount)
            .arg(meshData.nodeCount()));
    }

    std::unordered_set<int> stressElementIds;
    stressElementIds.reserve(datResult.stresses.size());
    for (const CalculiXElementStress &stress : datResult.stresses) {
        stressElementIds.insert(stress.elementId);
    }

    int matchedElementCount = 0;
    for (const TetraElement &element : meshData.tetraElements) {
        if (stressElementIds.find(element.id) != stressElementIds.end()) {
            ++matchedElementCount;
        }
    }
    if (matchedElementCount < meshData.tetraCount()) {
        warnings.append(QString("CSV export warning: only %1 of %2 tetrahedra have stress values; missing elements use 0.")
            .arg(matchedElementCount)
            .arg(meshData.tetraCount()));
    }

    return warnings;
}

bool writeElementStressCsv(
    const MeshData &meshData,
    const CalculiXDatResult &datResult,
    const QString &filePath,
    QString *errorMessage
)
{
    std::unordered_map<int, StressAccumulator> stressByElementId;
    stressByElementId.reserve(datResult.stresses.size());
    for (const CalculiXElementStress &stress : datResult.stresses) {
        StressAccumulator &accumulator = stressByElementId[stress.elementId];
        accumulator.vonMisesSum += CalculiXResultMath::vonMisesStress(stress);
        ++accumulator.count;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = "Open element stress CSV failed: " + file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setRealNumberNotation(QTextStream::ScientificNotation);
    stream.setRealNumberPrecision(12);
    stream << "elementId,VonMises\n";

    for (const TetraElement &element : meshData.tetraElements) {
        const auto stressIt = stressByElementId.find(element.id);
        const double value = stressIt == stressByElementId.end() || stressIt->second.count <= 0
            ? 0.0
            : stressIt->second.vonMisesSum / static_cast<double>(stressIt->second.count);
        stream << element.id << ',' << value << '\n';
    }

    return true;
}
}

ResultCsvExportResult ResultCsvExporter::exportResult(
    const ProjectModel &projectModel,
    const ResultObject &resultObject,
    const QString &nodeDisplacementCsvPath
) const
{
    ResultCsvExportResult exportResult;
    exportResult.nodeDisplacementCsvPath = nodeDisplacementCsvPath;
    exportResult.elementStressCsvPath = elementStressCsvPathFor(nodeDisplacementCsvPath);

    if (nodeDisplacementCsvPath.isEmpty()) {
        exportResult.errorMessage = "CSV export path is empty.";
        return exportResult;
    }
    if (!ensureParentDirectory(exportResult.nodeDisplacementCsvPath, &exportResult.errorMessage)
            || !ensureParentDirectory(exportResult.elementStressCsvPath, &exportResult.errorMessage)) {
        return exportResult;
    }

    const ResultDataLoadResult loaded = ResultDataLoader().loadCalculiXResult(projectModel, resultObject);
    exportResult.warnings.append(loaded.warnings);
    if (!loaded.success) {
        exportResult.errorMessage = loaded.errors.isEmpty()
            ? QString("CSV export failed: cannot load result data.")
            : "CSV export failed: " + loaded.errors.join("; ");
        return exportResult;
    }
    exportResult.warnings.append(coverageWarnings(loaded.meshData, loaded.datResult));

    if (!writeNodeDisplacementCsv(
            loaded.meshData,
            loaded.datResult,
            exportResult.nodeDisplacementCsvPath,
            &exportResult.errorMessage)) {
        return exportResult;
    }

    if (!writeElementStressCsv(
            loaded.meshData,
            loaded.datResult,
            exportResult.elementStressCsvPath,
            &exportResult.errorMessage)) {
        return exportResult;
    }

    exportResult.success = true;
    return exportResult;
}
