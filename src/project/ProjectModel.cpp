#include "project/ProjectModel.h"

void ProjectModel::clear()
{
    m_context.clear();
}

ProjectContext &ProjectModel::context()
{
    return m_context;
}

const ProjectContext &ProjectModel::context() const
{
    return m_context;
}

GeometryRepository &ProjectModel::geometryRepository()
{
    return m_context.geometry();
}

const GeometryRepository &ProjectModel::geometryRepository() const
{
    return m_context.geometry();
}

MeshRepository &ProjectModel::meshRepository()
{
    return m_context.mesh();
}

const MeshRepository &ProjectModel::meshRepository() const
{
    return m_context.mesh();
}

SolverRepository &ProjectModel::solverRepository()
{
    return m_context.solver();
}

const SolverRepository &ProjectModel::solverRepository() const
{
    return m_context.solver();
}

ResultRepository &ProjectModel::resultRepository()
{
    return m_context.results();
}

const ResultRepository &ProjectModel::resultRepository() const
{
    return m_context.results();
}

void ProjectModel::setProject(const Project &project)
{
    m_context.setProject(project);
}

const Project &ProjectModel::project() const
{
    return m_context.project();
}

bool ProjectModel::hasProject() const
{
    return m_context.hasProject();
}

QVector<GeometryObject> &ProjectModel::geometryObjects()
{
    return m_context.geometry().geometryObjects();
}

const QVector<GeometryObject> &ProjectModel::geometryObjects() const
{
    return m_context.geometry().geometryObjects();
}

QVector<BoxGeometry> &ProjectModel::boxes()
{
    return m_context.geometry().boxes();
}

const QVector<BoxGeometry> &ProjectModel::boxes() const
{
    return m_context.geometry().boxes();
}

QVector<CylinderGeometry> &ProjectModel::cylinders()
{
    return m_context.geometry().cylinders();
}

const QVector<CylinderGeometry> &ProjectModel::cylinders() const
{
    return m_context.geometry().cylinders();
}

QVector<SphereGeometry> &ProjectModel::spheres()
{
    return m_context.geometry().spheres();
}

const QVector<SphereGeometry> &ProjectModel::spheres() const
{
    return m_context.geometry().spheres();
}

QVector<MeshObject> &ProjectModel::meshObjects()
{
    return m_context.mesh().meshObjects();
}

const QVector<MeshObject> &ProjectModel::meshObjects() const
{
    return m_context.mesh().meshObjects();
}

std::vector<Material> &ProjectModel::materials()
{
    return m_context.solver().materials();
}

const std::vector<Material> &ProjectModel::materials() const
{
    return m_context.solver().materials();
}

std::vector<SectionAssignment> &ProjectModel::sectionAssignments()
{
    return m_context.solver().sectionAssignments();
}

const std::vector<SectionAssignment> &ProjectModel::sectionAssignments() const
{
    return m_context.solver().sectionAssignments();
}

std::vector<BoundaryCondition> &ProjectModel::boundaryConditions()
{
    return m_context.solver().boundaryConditions();
}

const std::vector<BoundaryCondition> &ProjectModel::boundaryConditions() const
{
    return m_context.solver().boundaryConditions();
}

std::vector<Load> &ProjectModel::loads()
{
    return m_context.solver().loads();
}

const std::vector<Load> &ProjectModel::loads() const
{
    return m_context.solver().loads();
}

std::vector<FaceGroup> &ProjectModel::faceGroups()
{
    return m_context.solver().faceGroups();
}

const std::vector<FaceGroup> &ProjectModel::faceGroups() const
{
    return m_context.solver().faceGroups();
}

void ProjectModel::ensureDefaultFaceGroups()
{
    m_context.solver().ensureDefaultFaceGroups(m_context.geometry());
}

const Selection &ProjectModel::selection() const
{
    return m_context.selectionState().current();
}

SelectionCapabilities ProjectModel::selectionCapabilities() const
{
    return m_context.selectionState().capabilities();
}

void ProjectModel::setSelection(const Selection &selection)
{
    m_context.selectionState().select(selection);
}

void ProjectModel::clearSelection()
{
    m_context.selectionState().clear();
}

void ProjectModel::clearSelectionIfKind(SelectionKind kind)
{
    m_context.selectionState().clearIfKind(kind);
}

void ProjectModel::clearSolverSelection()
{
    m_context.selectionState().clearSolverSelection();
}

const GeometryObject *ProjectModel::geometryForSelection() const
{
    return selection().kind == SelectionKind::Geometry ? findGeometryByName(selection().id) : nullptr;
}

const MeshObject *ProjectModel::meshForSelection() const
{
    return selection().kind == SelectionKind::Mesh ? findMeshByName(selection().id) : nullptr;
}

Material *ProjectModel::materialForSelection()
{
    return selection().kind == SelectionKind::Material ? findMaterialById(selection().id) : nullptr;
}

const Material *ProjectModel::materialForSelection() const
{
    return selection().kind == SelectionKind::Material ? findMaterialById(selection().id) : nullptr;
}

BoundaryCondition *ProjectModel::boundaryConditionForSelection()
{
    return selection().kind == SelectionKind::BoundaryCondition ? findBoundaryConditionById(selection().id) : nullptr;
}

const BoundaryCondition *ProjectModel::boundaryConditionForSelection() const
{
    return selection().kind == SelectionKind::BoundaryCondition ? findBoundaryConditionById(selection().id) : nullptr;
}

Load *ProjectModel::loadForSelection()
{
    return selection().kind == SelectionKind::Load ? findLoadById(selection().id) : nullptr;
}

const Load *ProjectModel::loadForSelection() const
{
    return selection().kind == SelectionKind::Load ? findLoadById(selection().id) : nullptr;
}

ResultObject *ProjectModel::resultForSelection()
{
    return selection().kind == SelectionKind::Result ? findResultById(selection().id) : nullptr;
}

const ResultObject *ProjectModel::resultForSelection() const
{
    return selection().kind == SelectionKind::Result ? findResultById(selection().id) : nullptr;
}

const GeometryObject *ProjectModel::findGeometryByName(const QString &name) const
{
    return m_context.geometry().findGeometryByName(name);
}

const BoxGeometry *ProjectModel::findBoxByName(const QString &name) const
{
    return m_context.geometry().findBoxByName(name);
}

const CylinderGeometry *ProjectModel::findCylinderByName(const QString &name) const
{
    return m_context.geometry().findCylinderByName(name);
}

const SphereGeometry *ProjectModel::findSphereByName(const QString &name) const
{
    return m_context.geometry().findSphereByName(name);
}

const MeshObject *ProjectModel::findMeshByName(const QString &name) const
{
    return m_context.mesh().findMeshByName(name);
}

FaceGroup *ProjectModel::findFaceGroupById(const QString &id)
{
    return m_context.solver().findFaceGroupById(id);
}

const FaceGroup *ProjectModel::findFaceGroupById(const QString &id) const
{
    return m_context.solver().findFaceGroupById(id);
}

Material *ProjectModel::findMaterialById(const QString &id)
{
    return m_context.solver().findMaterialById(id);
}

const Material *ProjectModel::findMaterialById(const QString &id) const
{
    return m_context.solver().findMaterialById(id);
}

SectionAssignment *ProjectModel::findSectionAssignmentById(const QString &id)
{
    return m_context.solver().findSectionAssignmentById(id);
}

const SectionAssignment *ProjectModel::findSectionAssignmentById(const QString &id) const
{
    return m_context.solver().findSectionAssignmentById(id);
}

BoundaryCondition *ProjectModel::findBoundaryConditionById(const QString &id)
{
    return m_context.solver().findBoundaryConditionById(id);
}

const BoundaryCondition *ProjectModel::findBoundaryConditionById(const QString &id) const
{
    return m_context.solver().findBoundaryConditionById(id);
}

Load *ProjectModel::findLoadById(const QString &id)
{
    return m_context.solver().findLoadById(id);
}

const Load *ProjectModel::findLoadById(const QString &id) const
{
    return m_context.solver().findLoadById(id);
}

ResultObject *ProjectModel::findResultById(const QString &id)
{
    return m_context.results().findResultById(id);
}

const ResultObject *ProjectModel::findResultById(const QString &id) const
{
    return m_context.results().findResultById(id);
}
