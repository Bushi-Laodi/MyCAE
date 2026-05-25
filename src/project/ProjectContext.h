#pragma once

#include "project/GeometryRepository.h"
#include "project/MeshRepository.h"
#include "project/Project.h"
#include "project/ResultRepository.h"
#include "project/SelectionState.h"
#include "project/SolverRepository.h"

class ProjectContext
{
public:
    void clear();

    Project &project();
    const Project &project() const;
    void setProject(const Project &project);
    bool hasProject() const;

    GeometryRepository &geometry();
    const GeometryRepository &geometry() const;
    MeshRepository &mesh();
    const MeshRepository &mesh() const;
    SolverRepository &solver();
    const SolverRepository &solver() const;
    ResultRepository &results();
    const ResultRepository &results() const;
    SelectionState &selectionState();
    const SelectionState &selectionState() const;

private:
    Project m_project;
    GeometryRepository m_geometryRepository;
    MeshRepository m_meshRepository;
    SolverRepository m_solverRepository;
    ResultRepository m_resultRepository;
    SelectionState m_selectionState;
};
