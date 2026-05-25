#pragma once

#include <QString>

enum class PickMode
{
    None,
    Vertex,
    Edge,
    Face,
    Body
};

inline QString pickModeName(PickMode mode)
{
    switch (mode) {
    case PickMode::Vertex:
        return "Vertex";
    case PickMode::Edge:
        return "Edge";
    case PickMode::Face:
        return "Face";
    case PickMode::Body:
        return "Body";
    case PickMode::None:
        return "None";
    }

    return "None";
}
