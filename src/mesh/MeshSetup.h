#pragma once

#include <QString>

enum class MeshElementType
{
    Tetra4,
    Tetra10
};

enum class GmshMeshAlgorithm3D
{
    Default,
    Delaunay,
    Frontal,
    HXT
};

struct MeshSetup
{
    MeshElementType elementType = MeshElementType::Tetra4;
    GmshMeshAlgorithm3D algorithm = GmshMeshAlgorithm3D::Default;
    double minimumSize = 0.0;
    double maximumSize = 1.0;
    QString meshSizeUnit = "mm";
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
    case MeshElementType::Tetra10:
        return "tetra10";
    }
    return "tetra4";
}

inline MeshElementType meshElementTypeFromString(const QString &value)
{
    return value == "tetra10" ? MeshElementType::Tetra10 : MeshElementType::Tetra4;
}

inline QString displayName(MeshElementType elementType)
{
    switch (elementType) {
    case MeshElementType::Tetra4:
        return "Tet4 - Linear tetrahedron";
    case MeshElementType::Tetra10:
        return "Tet10 - Quadratic tetrahedron";
    }
    return "Tet4 - Linear tetrahedron";
}

inline QString toString(GmshMeshAlgorithm3D algorithm)
{
    switch (algorithm) {
    case GmshMeshAlgorithm3D::Default:
        return "default";
    case GmshMeshAlgorithm3D::Delaunay:
        return "delaunay";
    case GmshMeshAlgorithm3D::Frontal:
        return "frontal";
    case GmshMeshAlgorithm3D::HXT:
        return "hxt";
    }
    return "default";
}

inline GmshMeshAlgorithm3D gmshMeshAlgorithm3DFromString(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == "delaunay") {
        return GmshMeshAlgorithm3D::Delaunay;
    }
    if (normalized == "frontal" || normalized == "frontal-delaunay") {
        return GmshMeshAlgorithm3D::Frontal;
    }
    if (normalized == "hxt") {
        return GmshMeshAlgorithm3D::HXT;
    }
    return GmshMeshAlgorithm3D::Default;
}

inline QString displayName(GmshMeshAlgorithm3D algorithm)
{
    switch (algorithm) {
    case GmshMeshAlgorithm3D::Default:
        return QString::fromUtf8(u8"默认");
    case GmshMeshAlgorithm3D::Delaunay:
        return "Delaunay";
    case GmshMeshAlgorithm3D::Frontal:
        return "Frontal-Delaunay";
    case GmshMeshAlgorithm3D::HXT:
        return "HXT";
    }
    return QString::fromUtf8(u8"默认");
}

inline int gmshOptionValue(GmshMeshAlgorithm3D algorithm)
{
    switch (algorithm) {
    case GmshMeshAlgorithm3D::Delaunay:
        return 1;
    case GmshMeshAlgorithm3D::Frontal:
        return 4;
    case GmshMeshAlgorithm3D::HXT:
        return 10;
    case GmshMeshAlgorithm3D::Default:
        break;
    }
    return 0;
}
