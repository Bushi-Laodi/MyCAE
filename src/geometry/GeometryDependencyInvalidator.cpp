#include "geometry/GeometryDependencyInvalidator.h"

#include "geometry/FaceGroup.h"
#include "mesh/MeshManager.h"
#include "mesh/MeshObject.h"
#include "project/ProjectModel.h"
#include "result/ResultManager.h"
#include "result/ResultObject.h"

QStringList GeometryDependencyInvalidator::markGeometryChanged(
    ProjectModel &projectModel,
    const QString &geometryName,
    const QString &reason
) const
{
    QStringList messages;
    if (!projectModel.hasProject() || geometryName.trimmed().isEmpty()) {
        return messages;
    }

    for (FaceGroup &faceGroup : projectModel.faceGroups()) {
        if (faceGroup.geometryName == geometryName) {
            faceGroup.needsReview = true;
            faceGroup.reviewReason = reason;
            messages.append("Face group marked for review: " + faceGroup.id);
        }
    }

    QStringList staleMeshNames;
    MeshManager meshManager(projectModel.project().rootPath);
    for (MeshObject &meshObject : projectModel.meshObjects()) {
        if (meshObject.sourceGeometryName != geometryName) {
            continue;
        }
        meshObject.stale = true;
        meshObject.staleReason = reason;
        staleMeshNames.append(meshObject.name);
        QString saveError;
        if (!meshManager.saveMeshObject(meshObject, &saveError)) {
            messages.append("Mesh stale state save failed: " + meshObject.name + " - " + saveError);
        } else {
            messages.append("Mesh marked stale: " + meshObject.name);
        }
    }

    bool resultIndexChanged = false;
    for (ResultObject &resultObject : projectModel.resultRepository().results()) {
        if (!staleMeshNames.contains(resultObject.meshName)) {
            continue;
        }
        resultObject.stale = true;
        resultObject.staleReason = reason;
        resultIndexChanged = true;
        messages.append("Result marked stale: " + resultObject.name);
    }

    if (resultIndexChanged) {
        QString saveError;
        if (!ResultManager().save(projectModel.project(), projectModel.resultRepository().results(), &saveError)) {
            messages.append("Result stale state save failed: " + saveError);
        }
    }

    return messages;
}
