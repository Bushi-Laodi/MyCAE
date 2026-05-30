#include "solver/calculix/CalculiXDatResultReader.h"

#include <QFile>
#include <QRegularExpression>
#include <QStringList>

namespace
{
enum class DatBlock
{
    None,
    Displacements,
    ReactionForces,
    Stresses
};

QStringList splitFields(const QString &line)
{
    static const QRegularExpression whitespace("\\s+");
    return line.trimmed().split(whitespace, Qt::SkipEmptyParts);
}

bool parseDisplacementLine(const QString &line, CalculiXNodeDisplacement &value)
{
    const QStringList fields = splitFields(line);
    if (fields.size() < 4) {
        return false;
    }

    bool okId = false;
    bool okX = false;
    bool okY = false;
    bool okZ = false;
    const int nodeId = fields.at(0).toInt(&okId);
    const double ux = fields.at(1).toDouble(&okX);
    const double uy = fields.at(2).toDouble(&okY);
    const double uz = fields.at(3).toDouble(&okZ);
    if (!okId || !okX || !okY || !okZ) {
        return false;
    }

    value.nodeId = nodeId;
    value.ux = ux;
    value.uy = uy;
    value.uz = uz;
    return true;
}

bool parseReactionForceLine(const QString &line, CalculiXNodeReactionForce &value)
{
    const QStringList fields = splitFields(line);
    if (fields.size() < 4) {
        return false;
    }

    bool okId = false;
    bool okX = false;
    bool okY = false;
    bool okZ = false;
    const int nodeId = fields.at(0).toInt(&okId);
    const double rf1 = fields.at(1).toDouble(&okX);
    const double rf2 = fields.at(2).toDouble(&okY);
    const double rf3 = fields.at(3).toDouble(&okZ);
    if (!okId || !okX || !okY || !okZ) {
        return false;
    }

    value.nodeId = nodeId;
    value.rf1 = rf1;
    value.rf2 = rf2;
    value.rf3 = rf3;
    return true;
}

bool parseStressLine(const QString &line, CalculiXElementStress &value)
{
    const QStringList fields = splitFields(line);
    if (fields.size() < 8) {
        return false;
    }

    bool okElement = false;
    bool okPoint = false;
    bool okSxx = false;
    bool okSyy = false;
    bool okSzz = false;
    bool okSxy = false;
    bool okSxz = false;
    bool okSyz = false;

    const int elementId = fields.at(0).toInt(&okElement);
    const int integrationPoint = fields.at(1).toInt(&okPoint);
    const double sxx = fields.at(2).toDouble(&okSxx);
    const double syy = fields.at(3).toDouble(&okSyy);
    const double szz = fields.at(4).toDouble(&okSzz);
    const double sxy = fields.at(5).toDouble(&okSxy);
    const double sxz = fields.at(6).toDouble(&okSxz);
    const double syz = fields.at(7).toDouble(&okSyz);

    if (!okElement || !okPoint || !okSxx || !okSyy || !okSzz || !okSxy || !okSxz || !okSyz) {
        return false;
    }

    value.elementId = elementId;
    value.integrationPoint = integrationPoint;
    value.sxx = sxx;
    value.syy = syy;
    value.szz = szz;
    value.sxy = sxy;
    value.sxz = sxz;
    value.syz = syz;
    return true;
}
}

CalculiXDatReadResult CalculiXDatResultReader::read(const QString &datFile) const
{
    CalculiXDatReadResult readResult;
    readResult.result.sourceFile = datFile;

    QFile file(datFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        readResult.errors.append("Cannot open CalculiX .dat result file: " + datFile);
        return readResult;
    }

    DatBlock block = DatBlock::None;
    bool blockHasData = false;
    while (!file.atEnd()) {
        const QString line = QString::fromLocal8Bit(file.readLine());
        const QString trimmed = line.trimmed();
        const QString lower = trimmed.toLower();

        if (lower.startsWith("displacements ")) {
            block = DatBlock::Displacements;
            blockHasData = false;
            continue;
        }
        if (lower.startsWith("forces ")) {
            block = DatBlock::ReactionForces;
            blockHasData = false;
            continue;
        }
        if (lower.startsWith("stresses ")) {
            block = DatBlock::Stresses;
            blockHasData = false;
            continue;
        }

        if (block == DatBlock::None) {
            continue;
        }
        if (trimmed.isEmpty()) {
            if (blockHasData) {
                block = DatBlock::None;
            }
            continue;
        }

        if (block == DatBlock::Displacements) {
            CalculiXNodeDisplacement displacement;
            if (parseDisplacementLine(trimmed, displacement)) {
                readResult.result.displacements.push_back(displacement);
                blockHasData = true;
            } else if (blockHasData) {
                block = DatBlock::None;
            }
            continue;
        }

        if (block == DatBlock::ReactionForces) {
            CalculiXNodeReactionForce reactionForce;
            if (parseReactionForceLine(trimmed, reactionForce)) {
                readResult.result.reactionForces.push_back(reactionForce);
                blockHasData = true;
            } else if (blockHasData) {
                block = DatBlock::None;
            }
            continue;
        }

        if (block == DatBlock::Stresses) {
            CalculiXElementStress stress;
            if (parseStressLine(trimmed, stress)) {
                readResult.result.stresses.push_back(stress);
                blockHasData = true;
            } else if (blockHasData) {
                block = DatBlock::None;
            }
        }
    }

    if (readResult.result.displacements.empty()) {
        readResult.errors.append("No displacement result block found in CalculiX .dat file.");
        return readResult;
    }
    if (readResult.result.stresses.empty()) {
        readResult.warnings.append("No stress result block found in CalculiX .dat file.");
    }

    readResult.success = true;
    return readResult;
}
