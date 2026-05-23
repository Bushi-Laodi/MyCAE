#pragma once

#include <QString>

enum class GeometryBooleanOperationType
{
    Union,
    Cut,
    Common
};

inline QString toStorageString(GeometryBooleanOperationType type)
{
    switch (type) {
    case GeometryBooleanOperationType::Union:
        return "union";
    case GeometryBooleanOperationType::Cut:
        return "cut";
    case GeometryBooleanOperationType::Common:
        return "common";
    }
    return "union";
}

inline QString toDisplayString(GeometryBooleanOperationType type)
{
    switch (type) {
    case GeometryBooleanOperationType::Union:
        return "Union";
    case GeometryBooleanOperationType::Cut:
        return "Cut";
    case GeometryBooleanOperationType::Common:
        return "Common";
    }
    return "Union";
}
