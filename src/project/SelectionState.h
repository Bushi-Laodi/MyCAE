#pragma once

#include <QString>

enum class SelectionKind
{
    None,
    Geometry,
    Mesh,
    FaceGroup,
    MaterialCategory,
    BoundaryConditionCategory,
    LoadCategory,
    SolverCategory,
    Material,
    BoundaryCondition,
    Load
};

struct SelectionCapabilities
{
    bool canGenerateMesh = false;
    bool canReadMeshInfo = false;
    bool canShowMesh = false;
    bool canEditSolverData = false;
    bool canDeleteSolverData = false;
};

struct Selection
{
    SelectionKind kind = SelectionKind::None;
    QString id;
    QString displayName;

    static Selection none();
    static Selection item(SelectionKind kind, const QString &id, const QString &displayName = {});
    static Selection category(SelectionKind kind);

    bool isNone() const;
    bool isSolverData() const;
    bool isSolverCategory() const;
    SelectionCapabilities capabilities() const;
};

class SelectionState
{
public:
    const Selection &current() const;
    SelectionCapabilities capabilities() const;
    void select(const Selection &selection);
    void clear();
    void clearIfKind(SelectionKind kind);
    void clearSolverSelection();

private:
    Selection m_current;
};
