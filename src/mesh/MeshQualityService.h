#pragma once

#include "mesh/MeshQualityChecker.h"

#include <QStringList>

struct MeshObject;

class MeshQualityService
{
public:
    static void applyReport(MeshObject &meshObject, const MeshQualityReport &report);
    static QStringList logMessages(const MeshQualityReport &report);
    static bool hasCriticalIssues(const MeshObject &meshObject);
    static bool hasWarningIssues(const MeshObject &meshObject);
    static QStringList solverPreflightMessages(const MeshObject &meshObject);
};
