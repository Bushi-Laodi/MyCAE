#include "solver/calculix/CalculiXLoadWriter.h"

#include <optional>

namespace
{
const CalculiXBoundaryExport *findBoundaryExport(
    const std::vector<CalculiXBoundaryExport> &boundaries,
    const QString &boundaryId
)
{
    for (const CalculiXBoundaryExport &boundary : boundaries) {
        if (boundary.boundaryId == boundaryId) {
            return &boundary;
        }
    }
    return nullptr;
}

std::optional<int> scalarForceDof(const CalculiXLoadData &load)
{
    const QString field = load.fieldName.trimmed().toLower();
    if (field == "x" || field == "fx" || field == "forcex" || field == "1") {
        return 1;
    }
    if (field == "y" || field == "fy" || field == "forcey" || field == "2") {
        return 2;
    }
    if (field == "z" || field == "fz" || field == "forcez" || field == "3") {
        return 3;
    }
    return std::nullopt;
}

bool appendConcentratedLoad(
    CalculiXInputDeck &deck,
    const CalculiXLoadData &load,
    const CalculiXBoundaryExport &boundary,
    QStringList &errors
)
{
    if (boundary.nodeIds.empty()) {
        errors.append("CalculiX export failed: load '" + load.name
            + "' has no mapped boundary node set.");
        return false;
    }

    const double nodeCount = static_cast<double>(boundary.nodeIds.size());
    deck.appendLine("*CLOAD");
    if (load.value.kind == LoadValueKind::Scalar) {
        const std::optional<int> dof = scalarForceDof(load);
        if (!dof.has_value()) {
            errors.append("CalculiX export failed: scalar force load '" + load.name
                + "' must set fieldName to X/Y/Z or 1/2/3.");
            return false;
        }
        for (const int nodeId : boundary.nodeIds) {
            deck.appendLine(QString("%1, %2, %3")
                .arg(nodeId)
                .arg(*dof)
                .arg(load.value.x / nodeCount, 0, 'g', 16));
        }
        return true;
    }

    for (const int nodeId : boundary.nodeIds) {
        if (load.value.x != 0.0) {
            deck.appendLine(QString("%1, 1, %2").arg(nodeId).arg(load.value.x / nodeCount, 0, 'g', 16));
        }
        if (load.value.y != 0.0) {
            deck.appendLine(QString("%1, 2, %2").arg(nodeId).arg(load.value.y / nodeCount, 0, 'g', 16));
        }
        if (load.value.z != 0.0) {
            deck.appendLine(QString("%1, 3, %2").arg(nodeId).arg(load.value.z / nodeCount, 0, 'g', 16));
        }
    }
    return true;
}

bool appendPressureLoad(
    CalculiXInputDeck &deck,
    const CalculiXLoadData &load,
    const CalculiXBoundaryExport &boundary,
    QStringList &errors
)
{
    if (boundary.surfaceFaces.empty()) {
        errors.append("CalculiX export failed: pressure load '" + load.name
            + "' has no mapped element surface.");
        return false;
    }
    if (load.value.kind != LoadValueKind::Scalar) {
        errors.append("CalculiX export failed: pressure load '" + load.name
            + "' must use a scalar value for *DLOAD export.");
        return false;
    }

    deck.appendLine("*DLOAD");
    deck.appendLine(QString("%1, P, %2")
        .arg(boundary.surfaceName)
        .arg(load.value.x, 0, 'g', 16));
    return true;
}
}

bool CalculiXLoadWriter::appendLoads(
    CalculiXInputDeck &deck,
    const CalculiXCaseData &caseData,
    const std::vector<CalculiXBoundaryExport> &boundaries,
    QStringList &errors
) const
{
    bool wroteLoad = false;
    for (const CalculiXLoadData &load : caseData.loads) {
        const CalculiXBoundaryExport *boundary = findBoundaryExport(boundaries, load.boundaryConditionId);
        if (!boundary) {
            errors.append("CalculiX export failed: load '" + load.name
                + "' has no mapped boundary export.");
            continue;
        }

        if (load.type == LoadType::Pressure) {
            wroteLoad = appendPressureLoad(deck, load, *boundary, errors) || wroteLoad;
        } else if (load.type == LoadType::BodyForce) {
            wroteLoad = appendConcentratedLoad(deck, load, *boundary, errors) || wroteLoad;
        } else {
            errors.append("CalculiX export failed: load '" + load.name
                + "' has unsupported CalculiX load type: " + toString(load.type));
        }
    }
    if (!wroteLoad && errors.isEmpty()) {
        errors.append("CalculiX export failed: no supported enabled load was written.");
    }
    return wroteLoad && errors.isEmpty();
}
