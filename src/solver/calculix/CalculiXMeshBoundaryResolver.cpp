#include "solver/calculix/CalculiXMeshBoundaryResolver.h"

#include "geometry/FaceGroup.h"
#include "project/ProjectModel.h"

#include <algorithm>
#include <optional>

namespace
{
bool hasMeshBoundaryForFaceGroup(const std::vector<MeshBoundary> &meshBoundaries, const QString &faceGroupId)
{
    return std::any_of(
        meshBoundaries.begin(),
        meshBoundaries.end(),
        [&faceGroupId](const MeshBoundary &meshBoundary) {
            return !faceGroupId.isEmpty() && meshBoundary.sourceFaceGroupId == faceGroupId;
        }
    );
}

int surfaceTriangleCountForTag(const MeshData &meshData, int tag)
{
    return static_cast<int>(std::count_if(
        meshData.surfaceTriangles.begin(),
        meshData.surfaceTriangles.end(),
        [tag](const SurfaceTriangleElement &triangle) {
            return triangle.physicalGroupTag == tag;
        }
    ));
}

std::vector<int> positiveUniqueTags(const std::vector<int> &tags)
{
    std::vector<int> result;
    for (const int tag : tags) {
        if (tag > 0) {
            result.push_back(tag);
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

QString tagListText(const std::vector<int> &tags)
{
    QStringList values;
    for (const int tag : tags) {
        values.append(QString::number(tag));
    }
    return values.join(", ");
}

bool matchesPhysicalGroup(const MeshPhysicalGroup &physicalGroup, const CalculiXBoundaryData &boundary)
{
    return physicalGroup.dimension == 2
        && (physicalGroup.name == boundary.faceGroupId || physicalGroup.name == boundary.faceGroupName);
}

std::optional<MeshBoundary> boundaryFromPhysicalGroup(
    const CalculiXCaseData &caseData,
    const CalculiXBoundaryData &boundary
)
{
    for (const MeshPhysicalGroup &physicalGroup : caseData.meshData.physicalGroups) {
        if (!matchesPhysicalGroup(physicalGroup, boundary)) {
            continue;
        }

        MeshBoundary meshBoundary;
        meshBoundary.id = caseData.meshName + "." + physicalGroup.name;
        meshBoundary.name = boundary.faceGroupName.isEmpty()
            ? FaceGroups::nameFromId(physicalGroup.name)
            : boundary.faceGroupName;
        meshBoundary.meshName = caseData.meshName;
        meshBoundary.sourceGeometryName = boundary.geometryName;
        meshBoundary.sourceFaceGroupId = boundary.faceGroupId;
        meshBoundary.physicalGroupName = physicalGroup.name;
        meshBoundary.physicalGroupTag = physicalGroup.tag;
        meshBoundary.physicalGroupTags = {physicalGroup.tag};
        meshBoundary.faceCount = surfaceTriangleCountForTag(caseData.meshData, physicalGroup.tag);
        return meshBoundary;
    }
    return std::nullopt;
}

std::optional<MeshBoundary> boundaryFromFaceGroupSurfaceTags(
    const ProjectModel &projectModel,
    const CalculiXCaseData &caseData,
    const CalculiXBoundaryData &boundary,
    QStringList &warnings,
    QStringList &logMessages
)
{
    const FaceGroup *faceGroup = projectModel.findFaceGroupById(boundary.faceGroupId);
    if (!faceGroup || faceGroup->faceIndices.empty()) {
        return std::nullopt;
    }

    std::vector<int> matchedTags;
    std::vector<int> missingTags;
    int faceCount = 0;
    for (const int tag : positiveUniqueTags(faceGroup->faceIndices)) {
        const int tagFaceCount = surfaceTriangleCountForTag(caseData.meshData, tag);
        if (tagFaceCount <= 0) {
            missingTags.push_back(tag);
            continue;
        }
        matchedTags.push_back(tag);
        faceCount += tagFaceCount;
    }

    if (matchedTags.empty()) {
        warnings.append("CalculiX boundary mapping failed: no mesh surface tag matched face group "
            + faceGroup->id + ".");
        return std::nullopt;
    }

    if (!missingTags.empty()) {
        warnings.append(QString("CalculiX boundary mapping partially matched face group %1; missing surface tag(s): %2.")
            .arg(faceGroup->id, tagListText(missingTags)));
    }

    MeshBoundary meshBoundary;
    meshBoundary.id = caseData.meshName + "." + faceGroup->id;
    meshBoundary.name = faceGroup->name;
    meshBoundary.meshName = caseData.meshName;
    meshBoundary.sourceGeometryName = faceGroup->geometryName;
    meshBoundary.sourceFaceGroupId = faceGroup->id;
    meshBoundary.physicalGroupName = faceGroup->id;
    meshBoundary.physicalGroupTag = matchedTags.front();
    meshBoundary.physicalGroupTags = matchedTags;
    meshBoundary.faceCount = faceCount;

    logMessages.append(QString("CalculiX boundary mapping derived from face group %1 using mesh surface tag(s): %2.")
        .arg(faceGroup->id, tagListText(matchedTags)));
    return meshBoundary;
}
}

CalculiXMeshBoundaryResolveResult CalculiXMeshBoundaryResolver::resolve(
    const ProjectModel &projectModel,
    const CalculiXCaseData &caseData
) const
{
    CalculiXMeshBoundaryResolveResult result;
    result.meshBoundaries = caseData.meshBoundaries;

    for (const CalculiXBoundaryData &boundary : caseData.boundaries) {
        if (boundary.faceGroupId.isEmpty()
                || hasMeshBoundaryForFaceGroup(result.meshBoundaries, boundary.faceGroupId)) {
            continue;
        }

        std::optional<MeshBoundary> resolvedBoundary = boundaryFromPhysicalGroup(caseData, boundary);
        if (resolvedBoundary.has_value()) {
            result.logMessages.append(
                "CalculiX boundary mapping restored from MSH physical group: " + boundary.faceGroupId
            );
            result.meshBoundaries.push_back(std::move(*resolvedBoundary));
            continue;
        }

        resolvedBoundary = boundaryFromFaceGroupSurfaceTags(
            projectModel,
            caseData,
            boundary,
            result.warnings,
            result.logMessages
        );
        if (resolvedBoundary.has_value()) {
            result.meshBoundaries.push_back(std::move(*resolvedBoundary));
        }
    }

    return result;
}
