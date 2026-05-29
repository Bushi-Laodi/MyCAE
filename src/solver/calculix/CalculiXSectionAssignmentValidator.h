#pragma once

#include "solver/SectionAssignment.h"

#include <QString>
#include <QStringList>

#include <vector>

struct Material;
struct MeshData;
struct MeshObject;
struct StructuralCase;

struct CalculiXSectionValidationItem
{
    SectionAssignment assignment;
    QString elementSetName;
    QString materialId;
    QString materialName;
    std::vector<int> elementIds;
};

struct CalculiXSectionValidationResult
{
    bool success = false;
    QStringList errors;
    QStringList warnings;
    std::vector<CalculiXSectionValidationItem> items;
};

class CalculiXSectionAssignmentValidator
{
public:
    static CalculiXSectionValidationResult validate(
        const StructuralCase &structuralCase,
        const MeshObject &meshObject,
        const MeshData &meshData
    );
};
