#include "solver/calculix/CalculiXResultGridBuilder.h"

#include "solver/calculix/CalculiXResultMath.h"

#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTetra.h>
#include <vtkUnstructuredGrid.h>

#include <limits>
#include <vector>
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

CalculiXResultGridBuildResult CalculiXResultGridBuilder::buildResultGrid(
    const MeshData &meshData,
    const CalculiXDatResult &result,
    const QString &fieldName,
    double deformationScale
) const
{
    CalculiXResultGridBuildResult buildResult;
    buildResult.scalarName = fieldName.isEmpty()
        ? QString(CalculiXResultFields::DisplacementMagnitude)
        : fieldName;
    buildResult.scalarAssociation = buildResult.scalarName == CalculiXResultFields::VonMisesStress
        ? CalculiXResultScalarAssociation::Cell
        : CalculiXResultScalarAssociation::Point;
    buildResult.meshNodeCount = meshData.nodeCount();
    buildResult.meshElementCount = meshData.tetraCount();

    if (meshData.nodes.empty() || meshData.tetraElements.empty()) {
        buildResult.errors.append("Cannot build result grid: mesh has no nodes or tetrahedra.");
        return buildResult;
    }
    if (result.displacements.empty() && isDisplacementField(buildResult.scalarName)) {
        buildResult.errors.append("Cannot build result grid: displacement field is empty.");
        return buildResult;
    }
    if (buildResult.scalarName == CalculiXResultFields::VonMisesStress && result.stresses.empty()) {
        buildResult.errors.append("Cannot build result grid: stress field is empty.");
        return buildResult;
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

    vtkNew<vtkPoints> points;
    vtkNew<vtkDoubleArray> displacementVector;
    vtkNew<vtkDoubleArray> uxArray;
    vtkNew<vtkDoubleArray> uyArray;
    vtkNew<vtkDoubleArray> uzArray;
    vtkNew<vtkDoubleArray> displacementMagnitudeArray;
    vtkNew<vtkDoubleArray> selectedPointScalar;
    vtkNew<vtkIntArray> nodeIdArray;
    displacementVector->SetName("Displacement");
    displacementVector->SetNumberOfComponents(3);
    uxArray->SetName(CalculiXResultFields::Ux);
    uyArray->SetName(CalculiXResultFields::Uy);
    uzArray->SetName(CalculiXResultFields::Uz);
    displacementMagnitudeArray->SetName(CalculiXResultFields::DisplacementMagnitude);
    selectedPointScalar->SetName(buildResult.scalarName.toUtf8().constData());
    nodeIdArray->SetName("MyCAE_NodeId");

    std::unordered_map<int, vtkIdType> nodeIdToVtkId;
    nodeIdToVtkId.reserve(meshData.nodes.size());

    double scalarMin = std::numeric_limits<double>::max();
    double scalarMax = std::numeric_limits<double>::lowest();

    for (const MeshNode &node : meshData.nodes) {
        const auto displacementIt = displacementByNodeId.find(node.id);
        if (displacementIt == displacementByNodeId.end()) {
            const vtkIdType vtkPointId = points->InsertNextPoint(node.x, node.y, node.z);
            nodeIdToVtkId.insert({node.id, vtkPointId});
            nodeIdArray->InsertNextValue(node.id);
            displacementVector->InsertNextTuple3(0.0, 0.0, 0.0);
            uxArray->InsertNextValue(0.0);
            uyArray->InsertNextValue(0.0);
            uzArray->InsertNextValue(0.0);
            displacementMagnitudeArray->InsertNextValue(0.0);
            if (isDisplacementField(buildResult.scalarName)) {
                selectedPointScalar->InsertNextValue(0.0);
                updateRange(0.0, scalarMin, scalarMax);
            }
            continue;
        }

        const CalculiXNodeDisplacement &displacement = displacementIt->second;
        const double magnitude = CalculiXResultMath::displacementMagnitude(displacement);
        const vtkIdType vtkPointId = points->InsertNextPoint(
            node.x + deformationScale * displacement.ux,
            node.y + deformationScale * displacement.uy,
            node.z + deformationScale * displacement.uz
        );
        nodeIdToVtkId.insert({node.id, vtkPointId});

        nodeIdArray->InsertNextValue(node.id);
        displacementVector->InsertNextTuple3(displacement.ux, displacement.uy, displacement.uz);
        uxArray->InsertNextValue(displacement.ux);
        uyArray->InsertNextValue(displacement.uy);
        uzArray->InsertNextValue(displacement.uz);
        displacementMagnitudeArray->InsertNextValue(magnitude);
        if (isDisplacementField(buildResult.scalarName)) {
            const double scalar = displacementScalar(displacement, buildResult.scalarName);
            selectedPointScalar->InsertNextValue(scalar);
            updateRange(scalar, scalarMin, scalarMax);
        }
        ++buildResult.matchedNodeCount;
    }

    auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);
    vtkNew<vtkDoubleArray> vonMisesArray;
    vtkNew<vtkIntArray> elementIdArray;
    vonMisesArray->SetName(CalculiXResultFields::VonMisesStress);
    elementIdArray->SetName("MyCAE_ElementId");

    for (const TetraElement &element : meshData.tetraElements) {
        const auto node1 = nodeIdToVtkId.find(element.node1);
        const auto node2 = nodeIdToVtkId.find(element.node2);
        const auto node3 = nodeIdToVtkId.find(element.node3);
        const auto node4 = nodeIdToVtkId.find(element.node4);
        if (node1 == nodeIdToVtkId.end()
                || node2 == nodeIdToVtkId.end()
                || node3 == nodeIdToVtkId.end()
                || node4 == nodeIdToVtkId.end()) {
            buildResult.errors.append(QString("Tetrahedron %1 references a missing node.").arg(element.id));
            return buildResult;
        }

        vtkNew<vtkTetra> tetra;
        tetra->GetPointIds()->SetId(0, node1->second);
        tetra->GetPointIds()->SetId(1, node2->second);
        tetra->GetPointIds()->SetId(2, node3->second);
        tetra->GetPointIds()->SetId(3, node4->second);
        grid->InsertNextCell(tetra->GetCellType(), tetra->GetPointIds());
        elementIdArray->InsertNextValue(element.id);

        double cellStress = 0.0;
        const auto stressIt = stressByElementId.find(element.id);
        if (stressIt != stressByElementId.end() && stressIt->second.count > 0) {
            cellStress = stressIt->second.vonMisesSum / static_cast<double>(stressIt->second.count);
            ++buildResult.matchedElementCount;
        }
        vonMisesArray->InsertNextValue(cellStress);
        if (buildResult.scalarName == CalculiXResultFields::VonMisesStress) {
            updateRange(cellStress, scalarMin, scalarMax);
        }
    }

    grid->GetPointData()->AddArray(displacementVector);
    grid->GetPointData()->AddArray(uxArray);
    grid->GetPointData()->AddArray(uyArray);
    grid->GetPointData()->AddArray(uzArray);
    grid->GetPointData()->AddArray(displacementMagnitudeArray);
    grid->GetPointData()->AddArray(nodeIdArray);
    if (isDisplacementField(buildResult.scalarName)
            && buildResult.scalarName != CalculiXResultFields::Ux
            && buildResult.scalarName != CalculiXResultFields::Uy
            && buildResult.scalarName != CalculiXResultFields::Uz
            && buildResult.scalarName != CalculiXResultFields::DisplacementMagnitude) {
        grid->GetPointData()->AddArray(selectedPointScalar);
    }
    grid->GetCellData()->AddArray(vonMisesArray);
    grid->GetCellData()->AddArray(elementIdArray);
    grid->GetPointData()->SetActiveVectors("Displacement");
    if (buildResult.scalarAssociation == CalculiXResultScalarAssociation::Point) {
        grid->GetPointData()->SetActiveScalars(buildResult.scalarName.toUtf8().constData());
    } else {
        grid->GetCellData()->SetActiveScalars(buildResult.scalarName.toUtf8().constData());
    }

    if (buildResult.matchedNodeCount < buildResult.meshNodeCount) {
        buildResult.warnings.append(QString("Only %1 of %2 mesh nodes have displacement values; missing nodes use 0.")
            .arg(buildResult.matchedNodeCount)
            .arg(buildResult.meshNodeCount));
    }
    if (buildResult.scalarName == CalculiXResultFields::VonMisesStress
            && buildResult.matchedElementCount < buildResult.meshElementCount) {
        buildResult.warnings.append(QString("Only %1 of %2 tetrahedra have stress values; missing cells use 0.")
            .arg(buildResult.matchedElementCount)
            .arg(buildResult.meshElementCount));
    }

    buildResult.scalarMin = scalarMin == std::numeric_limits<double>::max() ? 0.0 : scalarMin;
    buildResult.scalarMax = scalarMax == std::numeric_limits<double>::lowest() ? 0.0 : scalarMax;
    buildResult.grid = grid;
    buildResult.success = true;
    return buildResult;
}
