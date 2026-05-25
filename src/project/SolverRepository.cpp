#include "project/SolverRepository.h"

#include "project/GeometryRepository.h"

void SolverRepository::clear()
{
    clearSolverData();
    m_faceGroups.clear();
}

void SolverRepository::clearSolverData()
{
    m_materials.clear();
    m_boundaryConditions.clear();
    m_loads.clear();
}

std::vector<Material> &SolverRepository::materials()
{
    return m_materials;
}

const std::vector<Material> &SolverRepository::materials() const
{
    return m_materials;
}

std::vector<BoundaryCondition> &SolverRepository::boundaryConditions()
{
    return m_boundaryConditions;
}

const std::vector<BoundaryCondition> &SolverRepository::boundaryConditions() const
{
    return m_boundaryConditions;
}

std::vector<Load> &SolverRepository::loads()
{
    return m_loads;
}

const std::vector<Load> &SolverRepository::loads() const
{
    return m_loads;
}

std::vector<FaceGroup> &SolverRepository::faceGroups()
{
    return m_faceGroups;
}

const std::vector<FaceGroup> &SolverRepository::faceGroups() const
{
    return m_faceGroups;
}

void SolverRepository::ensureDefaultFaceGroups(const GeometryRepository &geometryRepository)
{
    for (const GeometryObject &geometry : geometryRepository.geometryObjects()) {
        const FaceGroup defaultFaceGroup = FaceGroups::makeDefault(geometry.name);
        bool exists = false;
        for (const FaceGroup &faceGroup : m_faceGroups) {
            if (faceGroup.id == defaultFaceGroup.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_faceGroups.push_back(defaultFaceGroup);
        }
    }
}

FaceGroup *SolverRepository::findFaceGroupById(const QString &id)
{
    for (FaceGroup &faceGroup : m_faceGroups) {
        if (faceGroup.id == id) {
            return &faceGroup;
        }
    }
    return nullptr;
}

const FaceGroup *SolverRepository::findFaceGroupById(const QString &id) const
{
    for (const FaceGroup &faceGroup : m_faceGroups) {
        if (faceGroup.id == id) {
            return &faceGroup;
        }
    }
    return nullptr;
}

Material *SolverRepository::findMaterialById(const QString &id)
{
    for (Material &material : m_materials) {
        if (material.id == id) {
            return &material;
        }
    }
    return nullptr;
}

const Material *SolverRepository::findMaterialById(const QString &id) const
{
    for (const Material &material : m_materials) {
        if (material.id == id) {
            return &material;
        }
    }
    return nullptr;
}

BoundaryCondition *SolverRepository::findBoundaryConditionById(const QString &id)
{
    for (BoundaryCondition &boundaryCondition : m_boundaryConditions) {
        if (boundaryCondition.id == id) {
            return &boundaryCondition;
        }
    }
    return nullptr;
}

const BoundaryCondition *SolverRepository::findBoundaryConditionById(const QString &id) const
{
    for (const BoundaryCondition &boundaryCondition : m_boundaryConditions) {
        if (boundaryCondition.id == id) {
            return &boundaryCondition;
        }
    }
    return nullptr;
}

Load *SolverRepository::findLoadById(const QString &id)
{
    for (Load &load : m_loads) {
        if (load.id == id) {
            return &load;
        }
    }
    return nullptr;
}

const Load *SolverRepository::findLoadById(const QString &id) const
{
    for (const Load &load : m_loads) {
        if (load.id == id) {
            return &load;
        }
    }
    return nullptr;
}
