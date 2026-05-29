#pragma once

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXCaseData.h"
#include "solver/calculix/CalculiXDeckTypes.h"
#include "solver/calculix/CalculiXInputDeck.h"

#include <QStringList>

#include <vector>

class CalculiXDeckSectionWriter
{
public:
    void appendHeading(CalculiXInputDeck &deck, const CalculiXCaseData &caseData) const;
    void appendMesh(CalculiXInputDeck &deck, const MeshData &meshData) const;
    void appendMaterial(CalculiXInputDeck &deck, const CalculiXMaterialData &material) const;
    void appendSolidSections(
        CalculiXInputDeck &deck,
        const std::vector<CalculiXSectionAssignmentData> &sectionAssignments
    ) const;
    void appendBoundaryDefinitions(
        CalculiXInputDeck &deck,
        const std::vector<CalculiXBoundaryExport> &boundaries
    ) const;
    bool appendFixedConstraints(
        CalculiXInputDeck &deck,
        const std::vector<CalculiXBoundaryExport> &boundaries,
        QStringList &errors
    ) const;
    void appendResultRequests(CalculiXInputDeck &deck) const;
};
