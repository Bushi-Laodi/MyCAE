#include "ProjectModelLoader.h"

#include "mesh/MeshManager.h"
#include "project/ProjectModel.h"

#include <vector>

ProjectModelLoader::ProjectModelLoader(const GeometryManager &geometryManager)
    : m_geometryManager(geometryManager)
{
}

bool ProjectModelLoader::loadGeometries(ProjectModel &projectModel, QString *errorMessage) const
{
    if (!m_geometryManager.loadBoxGeometries(projectModel.project(), &projectModel.boxes(), errorMessage)) {
        return false;
    }

    if (!m_geometryManager.loadCylinderGeometries(projectModel.project(), &projectModel.cylinders(), errorMessage)) {
        return false;
    }

    std::vector<GeometryObject> loadedGeometries;
    if (!m_geometryManager.loadGeometryObjects(projectModel.project(), loadedGeometries, errorMessage)) {
        return false;
    }

    projectModel.geometryObjects().clear();
    for (const GeometryObject &geometry : loadedGeometries) {
        projectModel.geometryObjects().append(geometry);
    }

    projectModel.clearSelectedGeometry();
    return true;
}

bool ProjectModelLoader::loadMeshes(ProjectModel &projectModel, QString *errorMessage) const
{
    projectModel.meshObjects().clear();

    MeshManager meshManager(projectModel.project().rootPath);
    std::vector<MeshObject> loadedMeshes;
    if (!meshManager.loadMeshObjects(loadedMeshes, errorMessage)) {
        return false;
    }

    for (const MeshObject &meshObject : loadedMeshes) {
        projectModel.meshObjects().append(meshObject);
    }

    projectModel.clearSelectedMesh();
    return true;
}
