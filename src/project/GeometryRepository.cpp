#include "project/GeometryRepository.h"

void GeometryRepository::clear()
{
    m_geometryObjects.clear();
    m_boxes.clear();
    m_cylinders.clear();
}

QVector<GeometryObject> &GeometryRepository::geometryObjects()
{
    return m_geometryObjects;
}

const QVector<GeometryObject> &GeometryRepository::geometryObjects() const
{
    return m_geometryObjects;
}

QVector<BoxGeometry> &GeometryRepository::boxes()
{
    return m_boxes;
}

const QVector<BoxGeometry> &GeometryRepository::boxes() const
{
    return m_boxes;
}

QVector<CylinderGeometry> &GeometryRepository::cylinders()
{
    return m_cylinders;
}

const QVector<CylinderGeometry> &GeometryRepository::cylinders() const
{
    return m_cylinders;
}

const GeometryObject *GeometryRepository::findGeometryByName(const QString &name) const
{
    for (const GeometryObject &geometry : m_geometryObjects) {
        if (geometry.name == name) {
            return &geometry;
        }
    }
    return nullptr;
}

const BoxGeometry *GeometryRepository::findBoxByName(const QString &name) const
{
    for (const BoxGeometry &box : m_boxes) {
        if (box.name == name) {
            return &box;
        }
    }
    return nullptr;
}

const CylinderGeometry *GeometryRepository::findCylinderByName(const QString &name) const
{
    for (const CylinderGeometry &cylinder : m_cylinders) {
        if (cylinder.name == name) {
            return &cylinder;
        }
    }
    return nullptr;
}
