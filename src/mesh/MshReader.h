#pragma once

#include "mesh/MeshData.h"

#include <QString>

class MshReader
{
public:
    static bool readMsh2(const QString &filePath, MeshData &meshData, QString *errorMessage = nullptr);
};
