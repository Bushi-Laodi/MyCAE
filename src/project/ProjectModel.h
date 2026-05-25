#pragma once

#include "project/ProjectContext.h"

#include <QString>
#include <QVector>

#include <vector>

class ProjectModel
{
public:
    void clear();

    ProjectContext &context();
    const ProjectContext &context() const;
    GeometryRepository &geometryRepository();
    const GeometryRepository &geometryRepository() const;
    MeshRepository &meshRepository();
    const MeshRepository &meshRepository() const;
    SolverRepository &solverRepository();
    const SolverRepository &solverRepository() const;

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
    std::vector<FaceGroup> &faceGroups();
    const std::vector<FaceGroup> &faceGroups() const;
    void ensureDefaultFaceGroups();

    const Selection &selection() const;
    SelectionCapabilities selectionCapabilities() const;
    void setSelection(const Selection &selection);
    void clearSelection();
    void clearSelectionIfKind(SelectionKind kind);
    void clearSolverSelection();
    const GeometryObject *geometryForSelection() const;
    const MeshObject *meshForSelection() const;
    Material *materialForSelection();
    const Material *materialForSelection() const;
    BoundaryCondition *boundaryConditionForSelection();
    const BoundaryCondition *boundaryConditionForSelection() const;
    Load *loadForSelection();
    const Load *loadForSelection() const;

    const GeometryObject *findGeometryByName(const QString &name) const;
    const BoxGeometry *findBoxByName(const QString &name) const;
    const CylinderGeometry *findCylinderByName(const QString &name) const;
    const MeshObject *findMeshByName(const QString &name) const;
    FaceGroup *findFaceGroupById(const QString &id);
    const FaceGroup *findFaceGroupById(const QString &id) const;
    Material *findMaterialById(const QString &id);
    const Material *findMaterialById(const QString &id) const;
    BoundaryCondition *findBoundaryConditionById(const QString &id);
    const BoundaryCondition *findBoundaryConditionById(const QString &id) const;
    Load *findLoadById(const QString &id);
    const Load *findLoadById(const QString &id) const;

private:
    ProjectContext m_context;
};
