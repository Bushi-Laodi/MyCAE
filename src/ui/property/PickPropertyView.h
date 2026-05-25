#pragma once

#include "picking/PickMode.h"

#include <QString>

#include <vector>

class QWidget;

class PickPropertyView
{
public:
    static void populate(
        QWidget *parent,
        PickMode mode,
        const QString &geometryName,
        const std::vector<int> &faceIndices
    );
};
