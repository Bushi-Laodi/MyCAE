#pragma once

#include <QString>
#include <QStringList>

#include <vector>

struct CalculiXNodeDisplacement
{
    int nodeId = 0;
    double ux = 0.0;
    double uy = 0.0;
    double uz = 0.0;
};

struct CalculiXElementStress
{
    int elementId = 0;
    int integrationPoint = 0;
    double sxx = 0.0;
    double syy = 0.0;
    double szz = 0.0;
    double sxy = 0.0;
    double sxz = 0.0;
    double syz = 0.0;
};

struct CalculiXNodeReactionForce
{
    int nodeId = 0;
    double rf1 = 0.0;
    double rf2 = 0.0;
    double rf3 = 0.0;
};

struct CalculiXDatResult
{
    QString sourceFile;
    std::vector<CalculiXNodeDisplacement> displacements;
    std::vector<CalculiXNodeReactionForce> reactionForces;
    std::vector<CalculiXElementStress> stresses;

    bool hasDisplacements() const
    {
        return !displacements.empty();
    }
};

struct CalculiXDatReadResult
{
    bool success = false;
    CalculiXDatResult result;
    QStringList warnings;
    QStringList errors;
};

class CalculiXDatResultReader
{
public:
    CalculiXDatReadResult read(const QString &datFile) const;
};
