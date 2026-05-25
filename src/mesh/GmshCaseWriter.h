#pragma once

#include <QString>
#include <QStringList>

#include <vector>

class ProjectModel;
struct FaceGroup;
struct GeometryObject;

struct GmshFaceGroupExportItem
{
    QString faceGroupId;
    QString displayName;
    QString geometryName;
    std::vector<int> faceIndices;
    bool physicalGroupEnabled = false;
    bool localMeshEnabled = false;
    double localMeshSize = 0.0;
    bool canMapToGmshSurface = false;
    QString reason;
};

struct GmshCaseWriterResult
{
    bool hasRequestedFaceGroupExport = false;
    bool canWritePhysicalGroups = false;
    QString meshInputFile;
    std::vector<GmshFaceGroupExportItem> items;
    QStringList logMessages;
    QStringList warnings;
    QStringList errors;
};

class GmshCaseWriter
{
public:
    GmshCaseWriterResult prepareFaceGroupExport(
        const ProjectModel &projectModel,
        const GeometryObject &geometry
    ) const;
};
