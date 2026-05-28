#pragma once

#include "geometry/FaceGroup.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"

#include <QString>
#include <QStringList>

#include <vector>

struct FaceGroupBindingSummary
{
    QString status;
    QStringList boundaryConditions;
    QStringList loads;
    QStringList warnings;
};

struct BoundaryConditionBindingSummary
{
    QString status;
    QString faceGroupDisplayName;
    int faceCount = 0;
    bool faceGroupExists = false;
    bool faceGroupIsEmpty = false;
    QStringList loads;
    QStringList warnings;
};

class BoundaryBindingInspector
{
public:
    static FaceGroupBindingSummary summarizeFaceGroup(
        const FaceGroup &faceGroup,
        const std::vector<BoundaryCondition> &boundaryConditions,
        const std::vector<Load> &loads
    );

    static BoundaryConditionBindingSummary summarizeBoundaryCondition(
        const BoundaryCondition &boundaryCondition,
        const std::vector<FaceGroup> &faceGroups,
        const std::vector<Load> &loads
    );
};
