#pragma once

#include "mesh/MeshBoundary.h"
#include "mesh/MeshData.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QString>
#include <QStringList>

#include <vector>

struct CalculiXMaterialData
{
    QString id;
    QString name;
    MaterialDomain domain = MaterialDomain::Fluid;
    double youngModulus = 0.0;
    double poissonRatio = 0.0;
    double density = 0.0;
};

struct CalculiXBoundaryData
{
    QString id;
    QString name;
    BoundaryConditionType type = BoundaryConditionType::Unknown;
    QString materialId;
    QString geometryName;
    QString faceGroupId;
    QString faceGroupName;
    QString meshBoundaryName;
};

struct CalculiXLoadData
{
    QString id;
    QString name;
    LoadType type = LoadType::Unknown;
    QString boundaryConditionId;
    QString fieldName;
    LoadValue value;
};

struct CalculiXCaseData
{
    QString caseName;
    QString meshName;
    QString meshFile;
    MeshData meshData;
    std::vector<MeshBoundary> meshBoundaries;
    std::vector<CalculiXMaterialData> materials;
    std::vector<CalculiXBoundaryData> boundaries;
    std::vector<CalculiXLoadData> loads;
};

struct CalculiXCaseDataBuildResult
{
    bool success = false;
    CalculiXCaseData caseData;
    QStringList warnings;
    QStringList errors;
    QStringList logMessages;
};
