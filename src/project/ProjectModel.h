#pragma once

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "project/Project.h"

#include <QString>
#include <QVector>

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

    void setSelectedGeometryName(const QString &name);
    const QString &selectedGeometryName() const;
    void clearSelectedGeometry();
    const GeometryObject *selectedGeometry() const;

    void setSelectedMeshName(const QString &name);
    const QString &selectedMeshName() const;
    void clearSelectedMesh();
    const MeshObject *selectedMesh() const;

    const GeometryObject *findGeometryByName(const QString &name) const;
    const BoxGeometry *findBoxByName(const QString &name) const;
    const CylinderGeometry *findCylinderByName(const QString &name) const;
    const MeshObject *findMeshByName(const QString &name) const;

private:
    Project m_project;
    QVector<GeometryObject> m_geometryObjects;
    QVector<BoxGeometry> m_boxes;
    QVector<CylinderGeometry> m_cylinders;
    QVector<MeshObject> m_meshObjects;
    QString m_selectedGeometryName;
    QString m_selectedMeshName;
};
