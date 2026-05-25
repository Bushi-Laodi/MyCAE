#pragma once

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/GeometryObject.h"

#include <QVector>

class GeometryRepository
{
public:
    void clear();

    QVector<GeometryObject> &geometryObjects();
    const QVector<GeometryObject> &geometryObjects() const;
    QVector<BoxGeometry> &boxes();
    const QVector<BoxGeometry> &boxes() const;
    QVector<CylinderGeometry> &cylinders();
    const QVector<CylinderGeometry> &cylinders() const;

    const GeometryObject *findGeometryByName(const QString &name) const;
    const BoxGeometry *findBoxByName(const QString &name) const;
    const CylinderGeometry *findCylinderByName(const QString &name) const;

private:
    QVector<GeometryObject> m_geometryObjects;
    QVector<BoxGeometry> m_boxes;
    QVector<CylinderGeometry> m_cylinders;
};
