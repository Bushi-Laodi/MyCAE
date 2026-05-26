#pragma once

#include <QString>

enum class MeshElementType
{
    Tetra4
};

struct MeshSetup
{
    MeshElementType elementType = MeshElementType::Tetra4;
    double minimumSize = 0.0;
    double maximumSize = 1.0;
    bool autoSize = true;
    QString localFaceGroupName;
    bool autoImportAfterGeneration = true;
    bool showBoundaryAfterImport = true;
};

inline QString toString(MeshElementType elementType)
{
    switch (elementType) {
    case MeshElementType::Tetra4:
        return "tetra4";
    }
    return "tetra4";
}

inline QString displayName(MeshElementType elementType)
{
    switch (elementType) {
    case MeshElementType::Tetra4:
        return "Tet4 - Linear tetrahedron";
    }
    return "Tet4 - Linear tetrahedron";
}
