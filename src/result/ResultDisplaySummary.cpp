#include "result/ResultDisplaySummary.h"

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDatResultReader.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"
#include "solver/calculix/CalculiXResultMath.h"

#include <algorithm>
#include <limits>
#include <unordered_map>

namespace
{
struct StressAccumulator
{
    double vonMisesSum = 0.0;
    int count = 0;
};

bool isDisplacementField(const QString &fieldName)
{
    return fieldName == CalculiXResultFields::Ux
        || fieldName == CalculiXResultFields::Uy
        || fieldName == CalculiXResultFields::Uz
        || fieldName == CalculiXResultFields::DisplacementMagnitude;
}

double displacementScalar(const CalculiXNodeDisplacement &value, const QString &fieldName)
{
    if (fieldName == CalculiXResultFields::Ux) {
        return value.ux;
    }
    if (fieldName == CalculiXResultFields::Uy) {
        return value.uy;
    }
    if (fieldName == CalculiXResultFields::Uz) {
        return value.uz;
    }
    return CalculiXResultMath::displacementMagnitude(value);
}

void updateRange(double value, double &scalarMin, double &scalarMax)
{
    scalarMin = std::min(scalarMin, value);
    scalarMax = std::max(scalarMax, value);
}
}

ResultDisplaySummary ResultDisplaySummarizer::summarize(
    const MeshData &meshData,
    const CalculiXDatResult &result,
    const QString &fieldName
) const
{
    ResultDisplaySummary summary;
    summary.scalarName = fieldName.isEmpty()
        ? QString(CalculiXResultFields::DisplacementMagnitude)
        : fieldName;
    summary.meshNodeCount = meshData.nodeCount();
    summary.meshElementCount = meshData.tetraCount();

    if (meshData.nodes.empty() || meshData.tetraCount() == 0) {
        summary.errors.append("Cannot build result summary: mesh has no nodes or tetrahedra.");
        return summary;
    }
    if (result.displacements.empty() && isDisplacementField(summary.scalarName)) {
        summary.errors.append("Cannot build result summary: displacement field is empty.");
        return summary;
    }
    if (summary.scalarName == CalculiXResultFields::VonMisesStress && result.stresses.empty()) {
        summary.errors.append("Cannot build result summary: stress field is empty.");
        return summary;
    }

    std::unordered_map<int, CalculiXNodeDisplacement> displacementByNodeId;
    displacementByNodeId.reserve(result.displacements.size());
    for (const CalculiXNodeDisplacement &displacement : result.displacements) {
        displacementByNodeId.insert_or_assign(displacement.nodeId, displacement);
    }

    std::unordered_map<int, StressAccumulator> stressByElementId;
    stressByElementId.reserve(result.stresses.size());
    for (const CalculiXElementStress &stress : result.stresses) {
        StressAccumulator &accumulator = stressByElementId[stress.elementId];
        accumulator.vonMisesSum += CalculiXResultMath::vonMisesStress(stress);
        ++accumulator.count;
    }

    double scalarMin = std::numeric_limits<double>::max();
    double scalarMax = std::numeric_limits<double>::lowest();

    for (const MeshNode &node : meshData.nodes) {
        const auto displacementIt = displacementByNodeId.find(node.id);
        if (displacementIt == displacementByNodeId.end()) {
            if (isDisplacementField(summary.scalarName)) {
                updateRange(0.0, scalarMin, scalarMax);
            }
            continue;
        }

        ++summary.matchedNodeCount;
        if (isDisplacementField(summary.scalarName)) {
            updateRange(displacementScalar(displacementIt->second, summary.scalarName), scalarMin, scalarMax);
        }
    }

    const auto updateElementStress = [&](int elementId) {
        double cellStress = 0.0;
        const auto stressIt = stressByElementId.find(elementId);
        if (stressIt != stressByElementId.end() && stressIt->second.count > 0) {
            cellStress = stressIt->second.vonMisesSum / static_cast<double>(stressIt->second.count);
            ++summary.matchedElementCount;
        }
        if (summary.scalarName == CalculiXResultFields::VonMisesStress) {
            updateRange(cellStress, scalarMin, scalarMax);
        }
    };
    for (const TetraElement &element : meshData.tetraElements) {
        updateElementStress(element.id);
    }
    for (const Tetra10Element &element : meshData.tetra10Elements) {
        updateElementStress(element.id);
    }

    if (summary.matchedNodeCount < summary.meshNodeCount) {
        summary.warnings.append(QString("Only %1 of %2 mesh nodes have displacement values; missing nodes use 0.")
            .arg(summary.matchedNodeCount)
            .arg(summary.meshNodeCount));
    }
    if (summary.scalarName == CalculiXResultFields::VonMisesStress
            && summary.matchedElementCount < summary.meshElementCount) {
        summary.warnings.append(QString("Only %1 of %2 tetrahedra have stress values; missing cells use 0.")
            .arg(summary.matchedElementCount)
            .arg(summary.meshElementCount));
    }

    summary.scalarMin = scalarMin == std::numeric_limits<double>::max() ? 0.0 : scalarMin;
    summary.scalarMax = scalarMax == std::numeric_limits<double>::lowest() ? 0.0 : scalarMax;
    summary.success = true;
    return summary;
}
