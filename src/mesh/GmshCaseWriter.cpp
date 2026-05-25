#include "mesh/GmshCaseWriter.h"

#include "geometry/FaceGroup.h"
#include "geometry/GeometryObject.h"
#include "project/ProjectModel.h"

namespace
{
bool requestsMeshExport(const FaceGroup &faceGroup)
{
    return faceGroup.physicalGroupEnabled || faceGroup.localMeshEnabled;
}

QString exportBlockReason(const FaceGroup &faceGroup)
{
    if (faceGroup.faceIndices.empty()) {
        return "face group has no picked face indices.";
    }
    if (faceGroup.faceReferences.empty()) {
        return "face group has no face references for future topology matching.";
    }
    return "stable OCC face index to Gmsh surface tag mapping is not implemented yet.";
}
}

GmshCaseWriterResult GmshCaseWriter::prepareFaceGroupExport(
    const ProjectModel &projectModel,
    const GeometryObject &geometry
) const
{
    GmshCaseWriterResult result;
    result.logMessages.append("Gmsh face group export scan: " + geometry.name);

    for (const FaceGroup &faceGroup : projectModel.solverRepository().faceGroups()) {
        if (faceGroup.geometryName != geometry.name || !requestsMeshExport(faceGroup)) {
            continue;
        }

        result.hasRequestedFaceGroupExport = true;

        GmshFaceGroupExportItem item;
        item.faceGroupId = faceGroup.id;
        item.displayName = FaceGroups::displayName(faceGroup);
        item.geometryName = faceGroup.geometryName;
        item.faceIndices = faceGroup.faceIndices;
        item.physicalGroupEnabled = faceGroup.physicalGroupEnabled;
        item.localMeshEnabled = faceGroup.localMeshEnabled;
        item.localMeshSize = faceGroup.localMeshSize;
        item.canMapToGmshSurface = false;
        item.reason = exportBlockReason(faceGroup);
        result.items.push_back(item);

        result.logMessages.append(
            QString("FaceGroup export deferred: %1, faces=%2, physicalGroup=%3, localMesh=%4, reason=%5")
                .arg(item.displayName)
                .arg(static_cast<int>(item.faceIndices.size()))
                .arg(item.physicalGroupEnabled ? "enabled" : "disabled")
                .arg(item.localMeshEnabled ? QString("enabled(size=%1)").arg(item.localMeshSize) : QString("disabled"))
                .arg(item.reason)
        );
    }

    if (!result.hasRequestedFaceGroupExport) {
        result.logMessages.append("No enabled FaceGroup physical/local mesh export requests found.");
        return result;
    }

    result.logMessages.append(
        "Gmsh physical groups and local mesh rules were not written: MyCAE must first map OCC face indices "
        "to Gmsh surface tags explicitly."
    );
    return result;
}
