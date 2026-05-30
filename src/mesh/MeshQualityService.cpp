#include "mesh/MeshQualityService.h"

#include "mesh/MeshObject.h"

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QStringList elementIdsToTextList(const std::vector<int> &elementIds)
{
    QStringList values;
    values.reserve(static_cast<int>(elementIds.size()));
    for (const int elementId : elementIds) {
        values.append(QString::number(elementId));
    }
    return values;
}
}

void MeshQualityService::applyReport(MeshObject &meshObject, const MeshQualityReport &report)
{
    meshObject.nodeCount = report.nodeCount;
    meshObject.tetraCount = report.tetraCount;
    meshObject.tetra4Count = report.tetra4Count;
    meshObject.tetra10Count = report.tetra10Count;
    meshObject.surfaceTriangleCount = report.surfaceTriangleCount;
    meshObject.qualityChecked = report.checked;
    meshObject.qualityStatus = report.status;
    meshObject.minimumEdgeLength = report.minimumEdgeLength;
    meshObject.maximumEdgeLength = report.maximumEdgeLength;
    meshObject.averageEdgeLength = report.averageEdgeLength;
    meshObject.minimumTetraVolume = report.minimumTetraVolume;
    meshObject.maximumTetraVolume = report.maximumTetraVolume;
    meshObject.averageTetraVolume = report.averageTetraVolume;
    meshObject.maximumAspectRatio = report.maximumAspectRatio;
    meshObject.averageAspectRatio = report.averageAspectRatio;
    meshObject.invalidTetraCount = report.invalidTetraCount;
    meshObject.degenerateTetraCount = report.degenerateTetraCount;
    meshObject.highAspectRatioTetraCount = report.highAspectRatioTetraCount;
    meshObject.qualityWarnings = report.warnings;
    meshObject.invalidElementIds = elementIdsToTextList(report.invalidElementIds);
    meshObject.degenerateElementIds = elementIdsToTextList(report.degenerateElementIds);
    meshObject.highAspectRatioElementIds = elementIdsToTextList(report.highAspectRatioElementIds);
}

QStringList MeshQualityService::logMessages(const MeshQualityReport &report)
{
    QStringList messages;
    messages.append(zh(u8"网格质量检查：") + report.status);
    messages.append(
        zh(u8"网格质量统计：nodes=%1，tetra=%2，tri=%3，minVol=%4，maxAspect=%5，invalid=%6，degenerate=%7，highAspect=%8")
            .arg(report.nodeCount)
            .arg(report.tetraCount)
            .arg(report.surfaceTriangleCount)
            .arg(report.minimumTetraVolume, 0, 'g', 8)
            .arg(report.maximumAspectRatio, 0, 'g', 8)
            .arg(report.invalidTetraCount)
            .arg(report.degenerateTetraCount)
            .arg(report.highAspectRatioTetraCount)
    );
    for (const QString &warning : report.warnings) {
        messages.append(zh(u8"网格质量警告：") + warning);
    }
    return messages;
}

bool MeshQualityService::hasCriticalIssues(const MeshObject &meshObject)
{
    if (!meshObject.qualityChecked) {
        return meshObject.nodeCount <= 0 || meshObject.tetraCount <= 0;
    }
    return meshObject.nodeCount <= 0
        || meshObject.tetraCount <= 0
        || meshObject.invalidTetraCount > 0
        || meshObject.degenerateTetraCount > 0
        || meshObject.minimumTetraVolume <= 0.0;
}

bool MeshQualityService::hasWarningIssues(const MeshObject &meshObject)
{
    return meshObject.highAspectRatioTetraCount > 0
        || meshObject.maximumAspectRatio > 20.0
        || !meshObject.qualityWarnings.isEmpty();
}

QStringList MeshQualityService::solverPreflightMessages(const MeshObject &meshObject)
{
    QStringList messages;
    if (!meshObject.qualityChecked) {
        messages.append(zh(u8"网格质量警告：当前网格尚未完成质量检查，建议先重新生成或读取网格信息。"));
        return messages;
    }

    if (hasCriticalIssues(meshObject)) {
        messages.append(zh(u8"网格质量错误：存在无效/退化单元或体积异常，已阻止求解。"));
    } else if (hasWarningIssues(meshObject)) {
        messages.append(zh(u8"网格质量警告：存在高长宽比单元，求解结果可能不稳定。"));
    }

    for (const QString &warning : meshObject.qualityWarnings) {
        messages.append(zh(u8"网格质量警告：") + warning);
    }
    return messages;
}
