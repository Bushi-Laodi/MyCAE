#pragma once

#include "result/ResultExtrema.h"

#include <QString>

struct CalculiXDatResult;
struct MeshData;

class ResultExtremaCalculator
{
public:
    ResultExtrema calculate(
        const MeshData &meshData,
        const CalculiXDatResult &result,
        const QString &selectedFieldName,
        double deformationScale
    ) const;
};
