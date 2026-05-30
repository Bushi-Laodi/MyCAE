#include "mesh/MeshQualityChecker.h"

#include <QHash>
#include <QtMath>

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace
{
struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

Vec3 toVec3(const MeshNode &node)
{
    return {node.x, node.y, node.z};
}

Vec3 subtract(const Vec3 &left, const Vec3 &right)
{
    return {left.x - right.x, left.y - right.y, left.z - right.z};
}

double dot(const Vec3 &left, const Vec3 &right)
{
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 cross(const Vec3 &left, const Vec3 &right)
{
    return {
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x
    };
}

double length(const Vec3 &vector)
{
    return qSqrt(dot(vector, vector));
}

double tetraVolume(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d)
{
    return qAbs(dot(subtract(a, d), cross(subtract(b, d), subtract(c, d)))) / 6.0;
}

std::array<int, 4> tetraCornerIds(const TetraElement &tetra)
{
    return {tetra.node1, tetra.node2, tetra.node3, tetra.node4};
}

std::array<int, 4> tetraCornerIds(const Tetra10Element &tetra)
{
    return {tetra.node1, tetra.node2, tetra.node3, tetra.node4};
}

template <typename Tetra>
bool cornerNodes(
    const Tetra &tetra,
    const QHash<int, MeshNode> &nodesById,
    std::array<Vec3, 4> &corners
)
{
    const std::array<int, 4> ids = tetraCornerIds(tetra);
    for (int index = 0; index < 4; ++index) {
        const auto it = nodesById.constFind(ids[index]);
        if (it == nodesById.constEnd()) {
            return false;
        }
        corners[index] = toVec3(it.value());
    }
    return true;
}

template <typename Tetra>
int tetraElementId(const Tetra &tetra)
{
    return tetra.id;
}

template <typename Tetra>
void accumulateTetraQuality(
    const Tetra &tetra,
    const QHash<int, MeshNode> &nodesById,
    MeshQualityReport &report,
    double &edgeLengthSum,
    int &edgeLengthCount,
    double &volumeSum,
    double &aspectRatioSum,
    int &validTetraCount
)
{
    std::array<Vec3, 4> corners;
    if (!cornerNodes(tetra, nodesById, corners)) {
        ++report.invalidTetraCount;
        report.invalidElementIds.push_back(tetraElementId(tetra));
        return;
    }

    const std::array<std::pair<int, int>, 6> edgePairs = {
        std::pair<int, int>{0, 1},
        std::pair<int, int>{0, 2},
        std::pair<int, int>{0, 3},
        std::pair<int, int>{1, 2},
        std::pair<int, int>{1, 3},
        std::pair<int, int>{2, 3}
    };

    double shortestEdge = std::numeric_limits<double>::max();
    double longestEdge = 0.0;
    for (const auto &edgePair : edgePairs) {
        const double edgeLength = length(subtract(corners[edgePair.first], corners[edgePair.second]));
        shortestEdge = std::min(shortestEdge, edgeLength);
        longestEdge = std::max(longestEdge, edgeLength);
        report.minimumEdgeLength = std::min(report.minimumEdgeLength, edgeLength);
        report.maximumEdgeLength = std::max(report.maximumEdgeLength, edgeLength);
        edgeLengthSum += edgeLength;
        ++edgeLengthCount;
    }

    const double volume = tetraVolume(corners[0], corners[1], corners[2], corners[3]);
    report.minimumTetraVolume = std::min(report.minimumTetraVolume, volume);
    report.maximumTetraVolume = std::max(report.maximumTetraVolume, volume);
    volumeSum += volume;

    const double localVolumeTolerance = longestEdge > 0.0
        ? qPow(longestEdge, 3.0) * 1.0e-12
        : 1.0e-18;
    if (shortestEdge <= 0.0 || volume <= localVolumeTolerance) {
        ++report.degenerateTetraCount;
        report.degenerateElementIds.push_back(tetraElementId(tetra));
    }

    const double aspectRatio = shortestEdge > 0.0
        ? longestEdge / shortestEdge
        : std::numeric_limits<double>::infinity();
    if (aspectRatio > 20.0) {
        ++report.highAspectRatioTetraCount;
        report.highAspectRatioElementIds.push_back(tetraElementId(tetra));
    }
    if (qIsFinite(aspectRatio)) {
        report.maximumAspectRatio = std::max(report.maximumAspectRatio, aspectRatio);
        aspectRatioSum += aspectRatio;
    }
    ++validTetraCount;
}

QString statusText(const MeshQualityReport &report)
{
    if (report.invalidTetraCount > 0 || report.degenerateTetraCount > 0) {
        return QString::fromUtf8(u8"需要检查");
    }
    if (report.highAspectRatioTetraCount > 0) {
        return QString::fromUtf8(u8"可用，有高长宽比单元");
    }
    return QString::fromUtf8(u8"通过");
}

void appendWarnings(MeshQualityReport &report)
{
    if (report.invalidTetraCount > 0) {
        report.warnings.append(QString::fromUtf8(u8"存在 %1 个引用缺失节点的四面体。").arg(report.invalidTetraCount));
    }
    if (report.degenerateTetraCount > 0) {
        report.warnings.append(QString::fromUtf8(u8"存在 %1 个零体积或近零体积四面体。").arg(report.degenerateTetraCount));
    }
    if (report.highAspectRatioTetraCount > 0) {
        report.warnings.append(QString::fromUtf8(u8"存在 %1 个长宽比大于 20 的四面体。").arg(report.highAspectRatioTetraCount));
    }
}
}

MeshQualityReport MeshQualityChecker::check(const MeshData &meshData)
{
    MeshQualityReport report;
    report.checked = true;
    report.nodeCount = meshData.nodeCount();
    report.tetra4Count = meshData.tetra4Count();
    report.tetra10Count = meshData.tetra10Count();
    report.tetraCount = meshData.tetraCount();
    report.surfaceTriangleCount = meshData.surfaceTriangleCount();
    report.minimumEdgeLength = std::numeric_limits<double>::max();
    report.minimumTetraVolume = std::numeric_limits<double>::max();

    QHash<int, MeshNode> nodesById;
    nodesById.reserve(static_cast<int>(meshData.nodes.size()));
    for (const MeshNode &node : meshData.nodes) {
        nodesById.insert(node.id, node);
    }

    double edgeLengthSum = 0.0;
    int edgeLengthCount = 0;
    double volumeSum = 0.0;
    double aspectRatioSum = 0.0;
    int validTetraCount = 0;

    for (const TetraElement &tetra : meshData.tetraElements) {
        accumulateTetraQuality(
            tetra,
            nodesById,
            report,
            edgeLengthSum,
            edgeLengthCount,
            volumeSum,
            aspectRatioSum,
            validTetraCount
        );
    }
    for (const Tetra10Element &tetra : meshData.tetra10Elements) {
        accumulateTetraQuality(
            tetra,
            nodesById,
            report,
            edgeLengthSum,
            edgeLengthCount,
            volumeSum,
            aspectRatioSum,
            validTetraCount
        );
    }

    if (edgeLengthCount == 0) {
        report.minimumEdgeLength = 0.0;
    } else {
        report.averageEdgeLength = edgeLengthSum / edgeLengthCount;
    }
    if (validTetraCount == 0) {
        report.minimumTetraVolume = 0.0;
    } else {
        report.averageTetraVolume = volumeSum / validTetraCount;
        report.averageAspectRatio = aspectRatioSum / validTetraCount;
    }

    appendWarnings(report);
    report.status = statusText(report);
    return report;
}
