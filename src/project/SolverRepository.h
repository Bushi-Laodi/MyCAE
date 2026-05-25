#pragma once

#include "geometry/FaceGroup.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <vector>

class GeometryRepository;

class SolverRepository
{
public:
    void clear();
    void clearSolverData();

    std::vector<Material> &materials();
    const std::vector<Material> &materials() const;
    std::vector<BoundaryCondition> &boundaryConditions();
    const std::vector<BoundaryCondition> &boundaryConditions() const;
    std::vector<Load> &loads();
    const std::vector<Load> &loads() const;
    std::vector<FaceGroup> &faceGroups();
    const std::vector<FaceGroup> &faceGroups() const;

    void ensureDefaultFaceGroups(const GeometryRepository &geometryRepository);

    FaceGroup *findFaceGroupById(const QString &id);
    const FaceGroup *findFaceGroupById(const QString &id) const;
    Material *findMaterialById(const QString &id);
    const Material *findMaterialById(const QString &id) const;
    BoundaryCondition *findBoundaryConditionById(const QString &id);
    const BoundaryCondition *findBoundaryConditionById(const QString &id) const;
    Load *findLoadById(const QString &id);
    const Load *findLoadById(const QString &id) const;

private:
    std::vector<Material> m_materials;
    std::vector<BoundaryCondition> m_boundaryConditions;
    std::vector<Load> m_loads;
    std::vector<FaceGroup> m_faceGroups;
};
