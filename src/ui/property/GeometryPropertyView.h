#pragma once

#include <QString>
#include <QStringList>

class QWidget;
struct GeometryObject;

struct GeometryPropertyDetails
{
    QString center;
    QString dimensions;
    QString brepFile;
    QString stepFile;
    bool visible = true;
    QStringList meshNames;
    QStringList staleMeshNames;
    QStringList faceGroupNames;
    QStringList reviewFaceGroupNames;
    QStringList boundaryConditionNames;
    QStringList loadNames;
    QStringList resultNames;
    QStringList staleResultNames;
};

class GeometryPropertyView
{
public:
    static void populate(
        QWidget *parent,
        const GeometryObject &geometry,
        const GeometryPropertyDetails &details = GeometryPropertyDetails{}
    );
};
