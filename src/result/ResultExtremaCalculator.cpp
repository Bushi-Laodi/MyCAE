#include "result/ResultExtremaCalculator.h"

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDatResultReader.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/calculix/CalculiXResultMath.h"

#include <unordered_map>

namespace
{
struct StressAccumulator
{
    double vonMisesSum = 0.0;
    int count = 0;
};

void updateNodeExtreme(
    ResultNodeExtreme &extreme,
    int nodeId,
    double x,
    double y,
    double z,
    double value,
    const QString &fieldName
)
{
    if (!extreme.valid || value > extreme.value) {
        extreme.valid = true;
        extreme.nodeId = nodeId;
        extreme.x = x;
        extreme.y = y;
        extreme.z = z;
        extreme.value = value;
        extreme.fieldName = fieldName;
    }
}

void updateElementExtreme(
    ResultElementExtreme &extreme,
    int elementId,
    double x,
    double y,
    double z,
    double value,
    const QString &fieldName
)
{
    if (!extreme.valid || value > extreme.value) {
        extreme.valid = true;
        extreme.elementId = elementId;
        extreme.x = x;
        extreme.y = y;
        extreme.z = z;
        extreme.value = value;
        extreme.fieldName = fieldName;
    }
}

bool centroidForElement(
    const TetraElement &element,
    const std::unordered_map<int, const MeshNode *> &nodesById,
    const std::unordered_map<int, CalculiXNodeDisplacement> *displacementsByNodeId,
    double deformationScale,
    double &x,
    double &y,
    double &z
)
{
    const int nodeIds[] = {element.node1, element.node2, element.node3, element.node4};
    x = 0.0;
    y = 0.0;
    z = 0.0;

    for (int nodeId : nodeIds) {
        const auto nodeIt = nodesById.find(nodeId);
        if (nodeIt == nodesById.end()) {
            return false;
        }

        double nodeX = nodeIt->second->x;
        double nodeY = nodeIt->second->y;
        double nodeZ = nodeIt->second->z;
        if (displacementsByNodeId && deformationScale != 0.0) {
            const auto displacementIt = displacementsByNodeId->find(nodeId);
            if (displacementIt != displacementsByNodeId->end()) {
                nodeX += deformationScale * displacementIt->second.ux;
                nodeY += deformationScale * displacementIt->second.uy;
                nodeZ += deformationScale * displacementIt->second.uz;
            }
        }

        x += nodeX;
        y += nodeY;
        z += nodeZ;
    }

    x /= 4.0;
    y /= 4.0;
    z /= 4.0;
    return true;
}

ResultExtremeMarker markerForNode(
    const ResultNodeExtreme &extreme,
    const std::unordered_map<int, CalculiXNodeDisplacement> &displacementsByNodeId,
    double deformationScale
)
{
    ResultExtremeMarker marker;
    if (!extreme.valid) {
        return marker;
    }

    marker.valid = true;
    marker.element = false;
    marker.id = extreme.nodeId;
    marker.x = extreme.x;
    marker.y = extreme.y;
    marker.z = extreme.z;
    marker.value = extreme.value;
    marker.fieldName = extreme.fieldName;

    const auto displacementIt = displacementsByNodeId.find(extreme.nodeId);
    if (displacementIt != displacementsByNodeId.end()) {
        marker.x += deformationScale * displacementIt->second.ux;
        marker.y += deformationScale * displacementIt->second.uy;
        marker.z += deformationScale * displacementIt->second.uz;
    }
    return marker;
}

ResultExtremeMarker markerForElement(
    const ResultElementExtreme &extreme,
    const MeshData &meshData,
    const std::unordered_map<int, const MeshNode *> &nodesById,
    const std::unordered_map<int, CalculiXNodeDisplacement> &displacementsByNodeId,
    double deformationScale
)
{
    ResultExtremeMarker marker;
    if (!extreme.valid) {
        return marker;
    }

    marker.valid = true;
    marker.element = true;
    marker.id = extreme.elementId;
    marker.x = extreme.x;
    marker.y = extreme.y;
    marker.z = extreme.z;
    marker.value = extreme.value;
    marker.fieldName = extreme.fieldName;

    if (deformationScale == 0.0) {
        return marker;
    }

    for (const TetraElement &element : meshData.tetraElements) {
        if (element.id != extreme.elementId) {
            continue;
        }

        centroidForElement(element, nodesById, &displacementsByNodeId, deformationScale, marker.x, marker.y, marker.z);
        return marker;
    }
    return marker;
}
}

ResultExtrema ResultExtremaCalculator::calculate(
    const MeshData &meshData,
    const CalculiXDatResult &result,
    const QString &selectedFieldName,
    double deformationScale
) const
{
    ResultExtrema extrema;

    std::unordered_map<int, const MeshNode *> nodesById;
    nodesById.reserve(meshData.nodes.size());
    for (const MeshNode &node : meshData.nodes) {
        nodesById.insert({node.id, &node});
    }

    std::unordered_map<int, CalculiXNodeDisplacement> displacementsByNodeId;
    displacementsByNodeId.reserve(result.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : result.displacements) {
        displacementsByNodeId.insert_or_assign(displacement.nodeId, displacement);
        const auto nodeIt = nodesById.find(displacement.nodeId);
        if (nodeIt == nodesById.end()) {
            continue;
        }

        const MeshNode &node = *nodeIt->second;
        updateNodeExtreme(extrema.maxUx, node.id, node.x, node.y, node.z, displacement.ux, CalculiXResultFields::Ux);
        updateNodeExtreme(extrema.maxUy, node.id, node.x, node.y, node.z, displacement.uy, CalculiXResultFields::Uy);
        updateNodeExtreme(extrema.maxUz, node.id, node.x, node.y, node.z, displacement.uz, CalculiXResultFields::Uz);
        updateNodeExtreme(
            extrema.maxDisplacementMagnitude,
            node.id,
            node.x,
            node.y,
            node.z,
            CalculiXResultMath::displacementMagnitude(displacement),
            CalculiXResultFields::DisplacementMagnitude
        );
    }

    std::unordered_map<int, StressAccumulator> stressByElementId;
    stressByElementId.reserve(result.stresses.size());
    for (const CalculiXElementStress &stress : result.stresses) {
        StressAccumulator &accumulator = stressByElementId[stress.elementId];
        accumulator.vonMisesSum += CalculiXResultMath::vonMisesStress(stress);
        ++accumulator.count;
    }

    for (const TetraElement &element : meshData.tetraElements) {
        const auto stressIt = stressByElementId.find(element.id);
        if (stressIt == stressByElementId.end() || stressIt->second.count <= 0) {
            continue;
        }

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        if (!centroidForElement(element, nodesById, nullptr, 0.0, x, y, z)) {
            continue;
        }

        updateElementExtreme(
            extrema.maxVonMisesStress,
            element.id,
            x,
            y,
            z,
            stressIt->second.vonMisesSum / static_cast<double>(stressIt->second.count),
            CalculiXResultFields::VonMisesStress
        );
    }

    if (selectedFieldName == CalculiXResultFields::Ux) {
        extrema.selectedMarker = markerForNode(extrema.maxUx, displacementsByNodeId, deformationScale);
    } else if (selectedFieldName == CalculiXResultFields::Uy) {
        extrema.selectedMarker = markerForNode(extrema.maxUy, displacementsByNodeId, deformationScale);
    } else if (selectedFieldName == CalculiXResultFields::Uz) {
        extrema.selectedMarker = markerForNode(extrema.maxUz, displacementsByNodeId, deformationScale);
    } else if (selectedFieldName == CalculiXResultFields::VonMisesStress) {
        extrema.selectedMarker =
            markerForElement(extrema.maxVonMisesStress, meshData, nodesById, displacementsByNodeId, deformationScale);
    } else {
        extrema.selectedMarker =
            markerForNode(extrema.maxDisplacementMagnitude, displacementsByNodeId, deformationScale);
    }

    return extrema;
}
