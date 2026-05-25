#pragma once

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDatResultReader.h"

#include <QString>
#include <QStringList>
#include <vtkSmartPointer.h>

class vtkUnstructuredGrid;

struct CalculiXResultGridBuildResult
{
    bool success = false;
    vtkSmartPointer<vtkUnstructuredGrid> grid;
    QString scalarName;
    int matchedNodeCount = 0;
    int meshNodeCount = 0;
    double scalarMin = 0.0;
    double scalarMax = 0.0;
    QStringList warnings;
    QStringList errors;
};

class CalculiXResultGridBuilder
{
public:
    CalculiXResultGridBuildResult buildDisplacementGrid(
        const MeshData &meshData,
        const CalculiXDatResult &result
    ) const;
};
