#include "solver/calculix/CalculiXInputDeckBuilder.h"

#include "solver/calculix/CalculiXBoundaryMapper.h"
#include "solver/calculix/CalculiXDeckSectionWriter.h"
#include "solver/calculix/CalculiXLoadWriter.h"

namespace
{
bool validateRequiredData(const CalculiXCaseData &caseData, CalculiXInputDeckBuildResult &result)
{
    if (caseData.meshData.nodes.empty()) {
        result.errors.append("CalculiX export failed: mesh contains no nodes.");
    }
    if (caseData.meshData.tetraElements.empty() && caseData.meshData.tetra10Elements.empty()) {
        result.errors.append("CalculiX export failed: mesh contains no supported tetrahedral elements.");
    }
    if (caseData.meshData.surfaceTriangles.empty()) {
        result.errors.append("CalculiX export failed: mesh contains no surface triangle elements.");
    }
    if (caseData.materials.empty()) {
        result.errors.append("CalculiX export failed: no material is defined.");
    }
    if (caseData.boundaries.empty()) {
        result.errors.append("CalculiX export failed: no enabled boundary condition is defined.");
    }
    if (caseData.loads.empty()) {
        result.errors.append("CalculiX export failed: no enabled load is defined.");
    }
    return result.errors.isEmpty();
}
}

CalculiXInputDeckBuildResult CalculiXInputDeckBuilder::build(const CalculiXCaseData &caseData) const
{
    CalculiXInputDeckBuildResult result;
    result.logMessages.append("CalculiX input deck build started.");
    if (!validateRequiredData(caseData, result)) {
        return result;
    }

    if (caseData.materials.size() > 1) {
        result.warnings.append("CalculiX export currently assigns the first material to all tetrahedral elements.");
    }

    const CalculiXBoundaryMapResult boundaryMapResult = CalculiXBoundaryMapper().map(caseData);
    result.warnings.append(boundaryMapResult.warnings);
    result.errors.append(boundaryMapResult.errors);
    if (!boundaryMapResult.success) {
        return result;
    }

    const CalculiXDeckSectionWriter sectionWriter;
    sectionWriter.appendHeading(result.deck, caseData);
    sectionWriter.appendMesh(result.deck, caseData.meshData);
    sectionWriter.appendMaterial(result.deck, caseData.materials.front());
    sectionWriter.appendBoundaryDefinitions(result.deck, boundaryMapResult.boundaries);
    if (!sectionWriter.appendFixedConstraints(result.deck, boundaryMapResult.boundaries, result.errors)) {
        return result;
    }

    result.deck.appendLine("*STEP");
    result.deck.appendLine("*STATIC");
    if (!CalculiXLoadWriter().appendLoads(result.deck, caseData, boundaryMapResult.boundaries, result.errors)) {
        return result;
    }

    sectionWriter.appendResultRequests(result.deck);
    result.success = true;
    result.logMessages.append("CalculiX input deck build finished.");
    return result;
}
