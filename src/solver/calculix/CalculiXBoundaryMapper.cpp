#include "solver/calculix/CalculiXBoundaryMapper.h"

#include "solver/calculix/CalculiXDeckFormatting.h"
#include "solver/calculix/CalculiXSurfaceMapper.h"

#include <algorithm>
#include <optional>

namespace
{
QString boundaryBaseName(const CalculiXBoundaryData &boundary)
{
    return boundary.id.isEmpty() ? boundary.name : boundary.id;
}

QString nodeSetName(const CalculiXBoundaryData &boundary)
{
    return "N_" + calculixSafeName(boundaryBaseName(boundary), "boundary");
}

QString elementSetName(const CalculiXBoundaryData &boundary)
{
    return "E_" + calculixSafeName(boundaryBaseName(boundary), "boundary");
}

QString surfaceName(const CalculiXBoundaryData &boundary)
{
    return "S_" + calculixSafeName(boundaryBaseName(boundary), "boundary");
}

bool hasLoadTarget(const CalculiXCaseData &caseData, const QString &boundaryId)
{
    return std::any_of(
        caseData.loads.begin(),
        caseData.loads.end(),
        [&boundaryId](const CalculiXLoadData &load) {
            return load.boundaryConditionId == boundaryId;
        }
    );
}

bool writesFixedConstraint(const CalculiXCaseData &caseData, const CalculiXBoundaryData &boundary)
{
    if (boundary.type == BoundaryConditionType::FixedSupport) {
        return true;
    }
    return boundary.type == BoundaryConditionType::Wall && !hasLoadTarget(caseData, boundary.id);
}

bool writesDisplacementConstraint(const CalculiXBoundaryData &boundary)
{
    return boundary.type == BoundaryConditionType::Displacement;
}

bool matchesBoundaryName(const QString &target, const MeshBoundary &meshBoundary)
{
    return !target.isEmpty()
        && (meshBoundary.name == target
            || meshBoundary.physicalGroupName == target
            || meshBoundary.id == target);
}

bool matchesFaceGroup(const CalculiXBoundaryData &boundary, const MeshBoundary &meshBoundary)
{
    return (!boundary.faceGroupId.isEmpty() && meshBoundary.sourceFaceGroupId == boundary.faceGroupId)
        || (!boundary.faceGroupName.isEmpty()
            && (meshBoundary.name == boundary.faceGroupName
                || meshBoundary.physicalGroupName == boundary.faceGroupName));
}

QString physicalGroupNameForTag(const MeshData &meshData, int tag)
{
    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroup.dimension == 2 && physicalGroup.tag == tag) {
            return physicalGroup.name;
        }
    }
    return {};
}

std::vector<int> tagsForMeshBoundary(const MeshBoundary &meshBoundary)
{
    std::vector<int> tags = meshBoundary.physicalGroupTags;
    if (tags.empty() && meshBoundary.physicalGroupTag >= 0) {
        tags.push_back(meshBoundary.physicalGroupTag);
    }
    std::sort(tags.begin(), tags.end());
    tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
    return tags;
}

std::vector<int> physicalTagsForBoundary(
    const MeshData &meshData,
    const std::vector<MeshBoundary> &meshBoundaries,
    const CalculiXBoundaryData &boundary
)
{
    for (const MeshBoundary &meshBoundary : meshBoundaries) {
        const bool matched = !boundary.meshBoundaryName.isEmpty()
            ? matchesBoundaryName(boundary.meshBoundaryName, meshBoundary)
            : matchesFaceGroup(boundary, meshBoundary);
        if (matched && meshBoundary.physicalGroupTag >= 0) {
            return tagsForMeshBoundary(meshBoundary);
        }
    }

    const QString targetName = boundary.meshBoundaryName.isEmpty()
        ? boundary.faceGroupName
        : boundary.meshBoundaryName;
    if (targetName.isEmpty()) {
        return {};
    }

    for (const MeshPhysicalGroup &physicalGroup : meshData.physicalGroups) {
        if (physicalGroup.dimension == 2 && physicalGroup.name == targetName) {
            return {physicalGroup.tag};
        }
    }
    return {};
}

std::vector<int> elementIdsForSurfaceFaces(const std::vector<CalculiXElementSurfaceFace> &surfaceFaces)
{
    std::vector<int> elementIds;
    elementIds.reserve(surfaceFaces.size());
    for (const CalculiXElementSurfaceFace &surfaceFace : surfaceFaces) {
        elementIds.push_back(surfaceFace.elementId);
    }
    std::sort(elementIds.begin(), elementIds.end());
    elementIds.erase(std::unique(elementIds.begin(), elementIds.end()), elementIds.end());
    return elementIds;
}

std::optional<CalculiXBoundaryExport> mapBoundary(
    const CalculiXCaseData &caseData,
    const CalculiXSurfaceMapper &surfaceMapper,
    const CalculiXBoundaryData &boundary,
    QStringList &warnings,
    QStringList &errors
)
{
    const bool isLoadBoundary = hasLoadTarget(caseData, boundary.id);
    const bool isFixedBoundary = writesFixedConstraint(caseData, boundary);
    const bool isDisplacementBoundary = writesDisplacementConstraint(boundary);
    if (!isLoadBoundary && !isFixedBoundary && !isDisplacementBoundary) {
        warnings.append("CalculiX export warning: boundary '" + boundary.name
            + "' is ignored by the structural solver because type '" + toString(boundary.type)
            + "' is neither a load target nor a supported structural constraint.");
        return std::nullopt;
    }

    const std::vector<int> physicalTags = physicalTagsForBoundary(
        caseData.meshData,
        caseData.meshBoundaries,
        boundary
    );
    if (physicalTags.empty()) {
        errors.append("CalculiX export failed: no mesh physical group is available for boundary '"
            + boundary.name + "'.");
        return std::nullopt;
    }

    std::vector<int> nodes = surfaceMapper.nodeSetForPhysicalTags(physicalTags);
    if (nodes.empty()) {
        errors.append("CalculiX export failed: mesh boundary '"
            + physicalGroupNameForTag(caseData.meshData, physicalTags.front())
            + "' has no surface nodes.");
        return std::nullopt;
    }

    std::vector<CalculiXElementSurfaceFace> faces = surfaceMapper.surfaceFacesForPhysicalTags(physicalTags);
    if (faces.empty()) {
        errors.append("CalculiX export failed: mesh boundary '"
            + physicalGroupNameForTag(caseData.meshData, physicalTags.front())
            + "' cannot be mapped to tetrahedral element faces.");
        return std::nullopt;
    }

    CalculiXBoundaryExport boundaryExport;
    boundaryExport.boundaryId = boundary.id;
    boundaryExport.boundaryName = boundary.name;
    boundaryExport.nodeSetName = nodeSetName(boundary);
    boundaryExport.elementSetName = elementSetName(boundary);
    boundaryExport.surfaceName = surfaceName(boundary);
    boundaryExport.nodeIds = std::move(nodes);
    boundaryExport.surfaceFaces = std::move(faces);
    boundaryExport.elementIds = elementIdsForSurfaceFaces(boundaryExport.surfaceFaces);
    boundaryExport.writesFixedConstraint = isFixedBoundary;
    boundaryExport.writesDisplacementConstraint = isDisplacementBoundary;
    boundaryExport.displacement = boundary.displacement;
    return boundaryExport;
}
}

CalculiXBoundaryMapResult CalculiXBoundaryMapper::map(const CalculiXCaseData &caseData) const
{
    CalculiXBoundaryMapResult result;
    const CalculiXSurfaceMapper surfaceMapper(caseData.meshData);
    for (const CalculiXBoundaryData &boundary : caseData.boundaries) {
        std::optional<CalculiXBoundaryExport> boundaryExport =
            mapBoundary(caseData, surfaceMapper, boundary, result.warnings, result.errors);
        if (boundaryExport.has_value()) {
            result.boundaries.push_back(std::move(*boundaryExport));
        }
    }
    result.success = result.errors.empty();
    return result;
}
