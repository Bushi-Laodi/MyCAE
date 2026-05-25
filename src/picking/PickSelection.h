#pragma once

#include "picking/PickMode.h"

#include <QString>

struct PickSelection
{
    PickMode mode = PickMode::None;
    QString geometryName;
    int topologyIndex = -1;
    long long cellId = -1;
    double worldX = 0.0;
    double worldY = 0.0;
    double worldZ = 0.0;
    double centerX = 0.0;
    double centerY = 0.0;
    double centerZ = 0.0;
    double normalX = 0.0;
    double normalY = 0.0;
    double normalZ = 0.0;
    double area = 0.0;

    bool isValid() const
    {
        return mode != PickMode::None && !geometryName.isEmpty() && topologyIndex > 0;
    }
};
