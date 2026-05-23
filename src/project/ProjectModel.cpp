#include "ProjectModel.h"

void ProjectModel::clear()
{
    m_project = {};
    m_geometryObjects.clear();
    m_boxes.clear();
    m_cylinders.clear();
    m_meshObjects.clear();
    m_materials.clear();
    m_boundaryConditions.clear();
    m_loads.clear();
    m_faceGroups.clear();
    m_selectedGeometryName.clear();
    m_selectedMeshName.clear();
    clearSelectedSolverData();
}

void ProjectModel::setProject(const Project &project)
{
    m_project = project;
    m_selectedGeometryName.clear();
    m_selectedMeshName.clear();
    clearSelectedSolverData();
}

const Project &ProjectModel::project() const
{
    return m_project;
}

bool ProjectModel::hasProject() const
{
    return !m_project.rootPath.isEmpty();
}

QVector<GeometryObject> &ProjectModel::geometryObjects()
{
    return m_geometryObjects;
}

const QVector<GeometryObject> &ProjectModel::geometryObjects() const
{
    return m_geometryObjects;
}

QVector<BoxGeometry> &ProjectModel::boxes()
{
    return m_boxes;
}

const QVector<BoxGeometry> &ProjectModel::boxes() const
{
    return m_boxes;
}

QVector<CylinderGeometry> &ProjectModel::cylinders()
{
    return m_cylinders;
}

const QVector<CylinderGeometry> &ProjectModel::cylinders() const
{
    return m_cylinders;
}

QVector<MeshObject> &ProjectModel::meshObjects()
{
    return m_meshObjects;
}

const QVector<MeshObject> &ProjectModel::meshObjects() const
{
    return m_meshObjects;
}

std::vector<Material> &ProjectModel::materials()
{
    return m_materials;
}

const std::vector<Material> &ProjectModel::materials() const
{
    return m_materials;
}

std::vector<BoundaryCondition> &ProjectModel::boundaryConditions()
{
    return m_boundaryConditions;
}

const std::vector<BoundaryCondition> &ProjectModel::boundaryConditions() const
{
    return m_boundaryConditions;
}

std::vector<Load> &ProjectModel::loads()
{
    return m_loads;
}

const std::vector<Load> &ProjectModel::loads() const
{
    return m_loads;
}

std::vector<FaceGroup> &ProjectModel::faceGroups()
{
    return m_faceGroups;
}

const std::vector<FaceGroup> &ProjectModel::faceGroups() const
{
    return m_faceGroups;
}

void ProjectModel::ensureDefaultFaceGroups()
{
    for (const GeometryObject &geometry : m_geometryObjects) {
        const QString defaultId = geometry.name + ".Default";
        bool exists = false;
        for (const FaceGroup &faceGroup : m_faceGroups) {
            if (faceGroup.id == defaultId) {
                exists = true;
                break;
            }
        }
        if (exists) {
            continue;
        }

        FaceGroup faceGroup;
        faceGroup.id = defaultId;
        faceGroup.name = "Default";
        faceGroup.geometryName = geometry.name;
        faceGroup.role = "Default";
        m_faceGroups.push_back(faceGroup);
    }
}

void ProjectModel::setSelectedGeometryName(const QString &name)
{
    m_selectedGeometryName = name;
    m_selectedMeshName.clear();
    clearSelectedSolverData();
}

const QString &ProjectModel::selectedGeometryName() const
{
    return m_selectedGeometryName;
}

void ProjectModel::clearSelectedGeometry()
{
    m_selectedGeometryName.clear();
}

const GeometryObject *ProjectModel::selectedGeometry() const
{
    return findGeometryByName(m_selectedGeometryName);
}

void ProjectModel::setSelectedMeshName(const QString &name)
{
    m_selectedMeshName = name;
    m_selectedGeometryName.clear();
    clearSelectedSolverData();
}

const QString &ProjectModel::selectedMeshName() const
{
    return m_selectedMeshName;
}

void ProjectModel::clearSelectedMesh()
{
    m_selectedMeshName.clear();
}

const MeshObject *ProjectModel::selectedMesh() const
{
    return findMeshByName(m_selectedMeshName);
}

void ProjectModel::setSelectedMaterialId(const QString &id)
{
    m_selectedMaterialId = id;
    m_selectedBoundaryConditionId.clear();
    m_selectedLoadId.clear();
    m_selectedGeometryName.clear();
    m_selectedMeshName.clear();
}

const QString &ProjectModel::selectedMaterialId() const
{
    return m_selectedMaterialId;
}

void ProjectModel::setSelectedBoundaryConditionId(const QString &id)
{
    m_selectedBoundaryConditionId = id;
    m_selectedMaterialId.clear();
    m_selectedLoadId.clear();
    m_selectedGeometryName.clear();
    m_selectedMeshName.clear();
}

const QString &ProjectModel::selectedBoundaryConditionId() const
{
    return m_selectedBoundaryConditionId;
}

void ProjectModel::setSelectedLoadId(const QString &id)
{
    m_selectedLoadId = id;
    m_selectedMaterialId.clear();
    m_selectedBoundaryConditionId.clear();
    m_selectedGeometryName.clear();
    m_selectedMeshName.clear();
}

const QString &ProjectModel::selectedLoadId() const
{
    return m_selectedLoadId;
}

void ProjectModel::clearSelectedSolverData()
{
    m_selectedMaterialId.clear();
    m_selectedBoundaryConditionId.clear();
    m_selectedLoadId.clear();
}

const GeometryObject *ProjectModel::findGeometryByName(const QString &name) const
{
    for (const GeometryObject &geometry : m_geometryObjects) {
        if (geometry.name == name) {
            return &geometry;
        }
    }
    return nullptr;
}

const BoxGeometry *ProjectModel::findBoxByName(const QString &name) const
{
    for (const BoxGeometry &box : m_boxes) {
        if (box.name == name) {
            return &box;
        }
    }
    return nullptr;
}

const CylinderGeometry *ProjectModel::findCylinderByName(const QString &name) const
{
    for (const CylinderGeometry &cylinder : m_cylinders) {
        if (cylinder.name == name) {
            return &cylinder;
        }
    }
    return nullptr;
}

const MeshObject *ProjectModel::findMeshByName(const QString &name) const
{
    for (const MeshObject &meshObject : m_meshObjects) {
        if (meshObject.name == name) {
            return &meshObject;
        }
    }
    return nullptr;
}

const FaceGroup *ProjectModel::findFaceGroupById(const QString &id) const
{
    for (const FaceGroup &faceGroup : m_faceGroups) {
        if (faceGroup.id == id) {
            return &faceGroup;
        }
    }
    return nullptr;
}

Material *ProjectModel::findMaterialById(const QString &id)
{
    for (Material &material : m_materials) {
        if (material.id == id) {
            return &material;
        }
    }
    return nullptr;
}

const Material *ProjectModel::findMaterialById(const QString &id) const
{
    for (const Material &material : m_materials) {
        if (material.id == id) {
            return &material;
        }
    }
    return nullptr;
}

BoundaryCondition *ProjectModel::findBoundaryConditionById(const QString &id)
{
    for (BoundaryCondition &boundaryCondition : m_boundaryConditions) {
        if (boundaryCondition.id == id) {
            return &boundaryCondition;
        }
    }
    return nullptr;
}

const BoundaryCondition *ProjectModel::findBoundaryConditionById(const QString &id) const
{
    for (const BoundaryCondition &boundaryCondition : m_boundaryConditions) {
        if (boundaryCondition.id == id) {
            return &boundaryCondition;
        }
    }
    return nullptr;
}

Load *ProjectModel::findLoadById(const QString &id)
{
    for (Load &load : m_loads) {
        if (load.id == id) {
            return &load;
        }
    }
    return nullptr;
}

const Load *ProjectModel::findLoadById(const QString &id) const
{
    for (const Load &load : m_loads) {
        if (load.id == id) {
            return &load;
        }
    }
    return nullptr;
}
