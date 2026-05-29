#pragma once

#include <QString>

struct SectionAssignment
{
    QString id;
    QString name;
    QString materialId;
    QString geometryName;
    QString meshName;
    QString elementSetName;
    bool enabled = true;
};

