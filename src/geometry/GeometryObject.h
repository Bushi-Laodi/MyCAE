#pragma once

#include <QString>

struct GeometryObject
{
    QString name;
    QString type;
    QString jsonFile;
    QString brepFile;
    QString stepFile;
    bool visible = true;
};
