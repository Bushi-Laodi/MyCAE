#pragma once

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/Project.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QString>
#include <QVector>

#include <vector>

class ProjectModel
{
public:
    void clear();

    void setProject(const Project &project);
    const Project &project() const;
    bool hasProject() const;

    QVector<GeometryObject> &geometryObjects();
    const QVector<GeometryObject> &geometryObjects() const;
    QVector<BoxGeometry> &boxes();
    const QVector<BoxGeometry> &boxes() const;
    QVector<CylinderGeometry> &cylinders();
    const QVector<CylinderGeometry> &cylinders() const;
    QVector<MeshObject> &meshObjects();
    const QVector<MeshObject> &meshObjects() const;
    std::vector<Material> &materials();
    const std::vector<Material> &materials() const;
    std::vector<BoundaryCondition> &boundaryConditions();
    const std::vector<BoundaryCondition> &boundaryConditions() const;
    std::vector<Load> &loads();
    const std::vector<Load> &loads() const;

    void setSelectedGeometryName(const QString &name);
    const QString &selectedGeometryName() const;
    void clearSelectedGeometry();
    const GeometryObject *selectedGeometry() const;

    void setSelectedMeshName(const QString &name);
    const QString &selectedMeshName() const;
    void clearSelectedMesh();
    const MeshObject *selectedMesh() const;

    void setSelectedMaterialId(const QString &id);
    const QString &selectedMaterialId() const;
    void setSelectedBoundaryConditionId(const QString &id);
    const QString &selectedBoundaryConditionId() const;
    void setSelectedLoadId(const QString &id);
    const QString &selectedLoadId() const;
    void clearSelectedSolverData();

    const GeometryObject *findGeometryByName(const QString &name) const;
    const BoxGeometry *findBoxByName(const QString &name) const;
    const CylinderGeometry *findCylinderByName(const QString &name) const;
    const MeshObject *findMeshByName(const QString &name) const;
    Material *findMaterialById(const QString &id);
    const Material *findMaterialById(const QString &id) const;
    BoundaryCondition *findBoundaryConditionById(const QString &id);
    const BoundaryCondition *findBoundaryConditionById(const QString &id) const;
    Load *findLoadById(const QString &id);
    const Load *findLoadById(const QString &id) const;

private:
    Project m_project;
    QVector<GeometryObject> m_geometryObjects;
    QVector<BoxGeometry> m_boxes;
    QVector<CylinderGeometry> m_cylinders;
    QVector<MeshObject> m_meshObjects;
    std::vector<Material> m_materials;
    std::vector<BoundaryCondition> m_boundaryConditions;
    std::vector<Load> m_loads;
    QString m_selectedGeometryName;
    QString m_selectedMeshName;
    QString m_selectedMaterialId;
    QString m_selectedBoundaryConditionId;
    QString m_selectedLoadId;
};
