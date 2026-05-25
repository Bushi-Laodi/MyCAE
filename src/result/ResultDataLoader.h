#pragma once

#include "mesh/MeshData.h"
#include "solver/calculix/CalculiXDatResultReader.h"

#include <QString>
#include <QStringList>

class ProjectModel;
struct ResultObject;

struct ResultDataLoadResult
{
    bool success = false;
    MeshData meshData;
    CalculiXDatResult datResult;
    QString meshFilePath;
    QString datFilePath;
    QStringList warnings;
    QStringList errors;
};

class ResultDataLoader
{
public:
    ResultDataLoadResult loadCalculiXResult(
        const ProjectModel &projectModel,
        const ResultObject &resultObject
    ) const;
};
