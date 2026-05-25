#include "project/SelectionState.h"

Selection Selection::none()
{
    return {};
}

Selection Selection::item(SelectionKind kind, const QString &id, const QString &displayName)
{
    Selection selection;
    selection.kind = kind;
    selection.id = id;
    selection.displayName = displayName.isEmpty() ? id : displayName;
    return selection;
}

Selection Selection::category(SelectionKind kind)
{
    Selection selection;
    selection.kind = kind;
    return selection;
}

bool Selection::isNone() const
{
    return kind == SelectionKind::None;
}

bool Selection::isSolverData() const
{
    return kind == SelectionKind::Material
        || kind == SelectionKind::BoundaryCondition
        || kind == SelectionKind::Load
        || kind == SelectionKind::Result;
}

bool Selection::isSolverCategory() const
{
    return kind == SelectionKind::MaterialCategory
        || kind == SelectionKind::BoundaryConditionCategory
        || kind == SelectionKind::LoadCategory
        || kind == SelectionKind::SolverCategory
        || kind == SelectionKind::ResultCategory;
}

SelectionCapabilities Selection::capabilities() const
{
    SelectionCapabilities capabilities;
    switch (kind) {
    case SelectionKind::Geometry:
        capabilities.canGenerateMesh = true;
        capabilities.canReadMeshInfo = true;
        capabilities.canShowMesh = true;
        break;
    case SelectionKind::Mesh:
        capabilities.canShowMesh = true;
        break;
    case SelectionKind::Material:
    case SelectionKind::BoundaryCondition:
    case SelectionKind::Load:
        capabilities.canEditSolverData = true;
        capabilities.canDeleteSolverData = true;
        break;
    case SelectionKind::Result:
        break;
    default:
        break;
    }
    return capabilities;
}

const Selection &SelectionState::current() const
{
    return m_current;
}

SelectionCapabilities SelectionState::capabilities() const
{
    return m_current.capabilities();
}

void SelectionState::select(const Selection &selection)
{
    m_current = selection;
}

void SelectionState::clear()
{
    m_current = Selection::none();
}

void SelectionState::clearIfKind(SelectionKind kind)
{
    if (m_current.kind == kind) {
        clear();
    }
}

void SelectionState::clearSolverSelection()
{
    if (m_current.isSolverData() || m_current.isSolverCategory()) {
        clear();
    }
}
