#pragma once

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDatResultReader.h"

#include <QString>
#include <QStringList>
#include <vtkSmartPointer.h>

class vtkUnstructuredGrid;

namespace CalculiXResultFields
{
constexpr const char *Ux = "Ux";
constexpr const char *Uy = "Uy";
constexpr const char *Uz = "Uz";
constexpr const char *DisplacementMagnitude = "Displacement Magnitude";
constexpr const char *VonMisesStress = "Von Mises Stress";
}

enum class CalculiXResultScalarAssociation
{
    Point,
    Cell
};

struct CalculiXResultGridBuildResult
{
    bool success = false;
    vtkSmartPointer<vtkUnstructuredGrid> grid;
    QString scalarName;
    CalculiXResultScalarAssociation scalarAssociation = CalculiXResultScalarAssociation::Point;
    int matchedNodeCount = 0;
    int meshNodeCount = 0;
    int matchedElementCount = 0;
    int meshElementCount = 0;
    double scalarMin = 0.0;
    double scalarMax = 0.0;
    QStringList warnings;
    QStringList errors;
};

class CalculiXResultGridBuilder
{
public:
    CalculiXResultGridBuildResult buildResultGrid(
        const MeshData &meshData,
        const CalculiXDatResult &result,
        const QString &fieldName,
        double deformationScale
    ) const;
};
