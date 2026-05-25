#include "project/ProjectContext.h"

void ProjectContext::clear()
{
    m_project = {};
    m_geometryRepository.clear();
    m_meshRepository.clear();
    m_solverRepository.clear();
    m_resultRepository.clear();
    m_selectionState.clear();
}

Project &ProjectContext::project()
{
    return m_project;
}

const Project &ProjectContext::project() const
{
    return m_project;
}

void ProjectContext::setProject(const Project &project)
{
    m_project = project;
    m_selectionState.clear();
}

bool ProjectContext::hasProject() const
{
    return !m_project.rootPath.isEmpty();
}

GeometryRepository &ProjectContext::geometry()
{
    return m_geometryRepository;
}

const GeometryRepository &ProjectContext::geometry() const
{
    return m_geometryRepository;
}

MeshRepository &ProjectContext::mesh()
{
    return m_meshRepository;
}

const MeshRepository &ProjectContext::mesh() const
{
    return m_meshRepository;
}

SolverRepository &ProjectContext::solver()
{
    return m_solverRepository;
}

const SolverRepository &ProjectContext::solver() const
{
    return m_solverRepository;
}

ResultRepository &ProjectContext::results()
{
    return m_resultRepository;
}

const ResultRepository &ProjectContext::results() const
{
    return m_resultRepository;
}

SelectionState &ProjectContext::selectionState()
{
    return m_selectionState;
}

const SelectionState &ProjectContext::selectionState() const
{
    return m_selectionState;
}
