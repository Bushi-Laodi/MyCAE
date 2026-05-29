#include "solver/calculix/CalculiXDeckSectionWriter.h"

#include "solver/calculix/CalculiXDeckFormatting.h"

#include <QStringList>

#include <algorithm>

namespace
{
void appendNodes(CalculiXInputDeck &deck, const MeshData &meshData)
{
    deck.appendLine("*NODE");
    for (const MeshNode &node : meshData.nodes) {
        deck.appendLine(QString("%1, %2, %3, %4")
            .arg(node.id)
            .arg(calculixNumber(node.x))
            .arg(calculixNumber(node.y))
            .arg(calculixNumber(node.z)));
    }
}

void appendAllNodeSet(CalculiXInputDeck &deck, const MeshData &meshData)
{
    std::vector<int> nodeIds;
    nodeIds.reserve(meshData.nodes.size());
    for (const MeshNode &node : meshData.nodes) {
        nodeIds.push_back(node.id);
    }
    std::sort(nodeIds.begin(), nodeIds.end());
    deck.appendLine("*NSET, NSET=NALL");
    appendCalculiXIdList(deck, nodeIds);
}

void appendElements(CalculiXInputDeck &deck, const MeshData &meshData)
{
    if (!meshData.tetraElements.empty()) {
        deck.appendLine("*ELEMENT, TYPE=C3D4, ELSET=EALL");
        for (const TetraElement &tetra : meshData.tetraElements) {
            deck.appendLine(QString("%1, %2, %3, %4, %5")
                .arg(tetra.id)
                .arg(tetra.node1)
                .arg(tetra.node2)
                .arg(tetra.node3)
                .arg(tetra.node4));
        }
    }
    if (!meshData.tetra10Elements.empty()) {
        deck.appendLine("*ELEMENT, TYPE=C3D10, ELSET=EALL");
        for (const Tetra10Element &tetra : meshData.tetra10Elements) {
            deck.appendLine(QStringList{
                QString::number(tetra.id),
                QString::number(tetra.node1),
                QString::number(tetra.node2),
                QString::number(tetra.node3),
                QString::number(tetra.node4),
                QString::number(tetra.node5),
                QString::number(tetra.node6),
                QString::number(tetra.node7),
                QString::number(tetra.node8),
                QString::number(tetra.node10),
                QString::number(tetra.node9)
            }.join(", "));
        }
    }
}
}

void CalculiXDeckSectionWriter::appendHeading(
    CalculiXInputDeck &deck,
    const CalculiXCaseData &caseData
) const
{
    deck.appendLine("*HEADING");
    deck.appendLine("MyCAE CalculiX input deck: " + calculixSafeName(caseData.caseName, "simulation_case"));
    deck.appendComment(QString("Nodes: %1, tetra4 elements: %2, tetra10 elements: %3, surface triangles: %4")
        .arg(caseData.meshData.nodeCount())
        .arg(caseData.meshData.tetra4Count())
        .arg(caseData.meshData.tetra10Count())
        .arg(caseData.meshData.surfaceTriangleCount()));
}

void CalculiXDeckSectionWriter::appendMesh(CalculiXInputDeck &deck, const MeshData &meshData) const
{
    appendNodes(deck, meshData);
    appendAllNodeSet(deck, meshData);
    appendElements(deck, meshData);
}

void CalculiXDeckSectionWriter::appendMaterial(
    CalculiXInputDeck &deck,
    const CalculiXMaterialData &material
) const
{
    const QString materialName = calculixSafeName(
        material.id.trimmed().isEmpty() ? material.name : material.id,
        "material_1"
    );
    deck.appendLine("*MATERIAL, NAME=" + materialName);
    deck.appendLine("*ELASTIC");
    deck.appendLine(QString("%1, %2")
        .arg(calculixNumber(material.youngModulus))
        .arg(calculixNumber(material.poissonRatio)));
    if (material.density > 0.0) {
        deck.appendLine("*DENSITY");
        deck.appendLine(calculixNumber(material.density));
    }
}

void CalculiXDeckSectionWriter::appendSolidSections(
    CalculiXInputDeck &deck,
    const std::vector<CalculiXSectionAssignmentData> &sectionAssignments
) const
{
    for (const CalculiXSectionAssignmentData &sectionAssignment : sectionAssignments) {
        if (!sectionAssignment.enabled) {
            continue;
        }

        const QString elementSetName = calculixSafeName(sectionAssignment.elementSetName, "EALL");
        if (elementSetName.compare("EALL", Qt::CaseInsensitive) != 0) {
            deck.appendLine("*ELSET, ELSET=" + elementSetName);
            appendCalculiXIdList(deck, sectionAssignment.elementIds);
        }

        const QString materialName = calculixSafeName(sectionAssignment.materialName, "material_1");
        deck.appendLine("*SOLID SECTION, ELSET=" + elementSetName + ", MATERIAL=" + materialName);
    }
}

void CalculiXDeckSectionWriter::appendBoundaryDefinitions(
    CalculiXInputDeck &deck,
    const std::vector<CalculiXBoundaryExport> &boundaries
) const
{
    for (const CalculiXBoundaryExport &boundary : boundaries) {
        deck.appendLine("*NSET, NSET=" + boundary.nodeSetName);
        appendCalculiXIdList(deck, boundary.nodeIds);
    }
    for (const CalculiXBoundaryExport &boundary : boundaries) {
        deck.appendLine("*ELSET, ELSET=" + boundary.elementSetName);
        appendCalculiXIdList(deck, boundary.elementIds);
    }
    for (const CalculiXBoundaryExport &boundary : boundaries) {
        deck.appendLine("*SURFACE, NAME=" + boundary.surfaceName + ", TYPE=ELEMENT");
        for (const CalculiXElementSurfaceFace &surfaceFace : boundary.surfaceFaces) {
            deck.appendLine(QString("%1, %2").arg(surfaceFace.elementId).arg(surfaceFace.faceLabel));
        }
    }
}

bool CalculiXDeckSectionWriter::appendFixedConstraints(
    CalculiXInputDeck &deck,
    const std::vector<CalculiXBoundaryExport> &boundaries,
    QStringList &errors
) const
{
    bool wroteConstraint = false;
    for (const CalculiXBoundaryExport &boundary : boundaries) {
        if (!boundary.writesFixedConstraint) {
            continue;
        }
        deck.appendLine("*BOUNDARY");
        deck.appendLine(boundary.nodeSetName + ", 1, 3, 0.0");
        wroteConstraint = true;
    }
    if (!wroteConstraint) {
        errors.append("CalculiX export failed: no fixed boundary constraint was written.");
    }
    return wroteConstraint;
}

void CalculiXDeckSectionWriter::appendResultRequests(CalculiXInputDeck &deck) const
{
    deck.appendLine("*NODE FILE");
    deck.appendLine("U");
    deck.appendLine("*EL FILE");
    deck.appendLine("S");
    deck.appendLine("*NODE PRINT, NSET=NALL");
    deck.appendLine("U");
    deck.appendLine("*EL PRINT, ELSET=EALL");
    deck.appendLine("S");
    deck.appendLine("*END STEP");
}
