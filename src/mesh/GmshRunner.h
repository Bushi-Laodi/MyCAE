#pragma once

#include "mesh/MeshSetup.h"

#include <QString>

struct GmshRunResult
{
    bool success = false;
    int exitCode = -1;
    QString standardOutput;
    QString standardError;
    QString errorMessage;
};

class GmshRunner
{
public:
    explicit GmshRunner(QString gmshExecutablePath = defaultGmshExecutablePath());

    static QString defaultGmshExecutablePath();

    QString gmshExecutablePath() const;
    GmshRunResult checkVersion() const;
    GmshRunResult generate3DMesh(
        const QString &inputGeometryFile,
        const QString &outputMeshFile,
        const MeshSetup &meshSetup = {}
    ) const;

private:
    QString m_gmshExecutablePath;
};
