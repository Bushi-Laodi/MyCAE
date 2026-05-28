#pragma once

#include "geometry/BoxGeometry.h"
#include "geometry/CylinderGeometry.h"
#include "geometry/GeometryBooleanOperation.h"
#include "geometry/GeometryObject.h"
#include "geometry/GeometryTransform.h"
#include "geometry/SphereGeometry.h"
#include "project/Project.h"

#include <QVector>

#include <vector>

class QString;

class GeometryManager
{
public:
    bool createDefaultBox(const Project &project, BoxGeometry *box, QString *errorMessage) const;
    bool createBox(const Project &project, const BoxGeometry &parameters, BoxGeometry *box, QString *errorMessage) const;
    bool createCylinder(const Project &project, const CylinderGeometry &parameters, CylinderGeometry *cylinder, QString *errorMessage) const;
    bool createSphere(const Project &project, const SphereGeometry &parameters, SphereGeometry *sphere, QString *errorMessage) const;
    bool createBooleanGeometry(
        const Project &project,
        const GeometryObject &leftGeometry,
        const GeometryObject &rightGeometry,
        GeometryBooleanOperationType operationType,
        const QString &requestedName,
        GeometryObject *geometry,
        QString *errorMessage
    ) const;
    bool importStepGeometry(
        const Project &project,
        const QString &sourceStepFile,
        GeometryObject *geometry,
        QString *errorMessage
    ) const;
    bool deleteGeometry(
        const Project &project,
        const GeometryObject &geometry,
        QStringList *deletedFiles,
        QString *errorMessage
    ) const;
    bool setGeometryVisible(
        const Project &project,
        const GeometryObject &geometry,
        bool visible,
        QString *errorMessage
    ) const;
    bool geometryCenter(
        const Project &project,
        const GeometryObject &geometry,
        GeometryCenter *center,
        QString *errorMessage
    ) const;
    bool transformGeometry(
        const Project &project,
        const GeometryObject &geometry,
        const GeometryTransformParameters &parameters,
        GeometryObject *transformedGeometry,
        QStringList *logMessages,
        QString *errorMessage
    ) const;
    bool loadBoxGeometries(const Project &project, QVector<BoxGeometry> *boxes, QString *errorMessage) const;
    bool loadCylinderGeometries(const Project &project, QVector<CylinderGeometry> *cylinders, QString *errorMessage) const;
    bool loadSphereGeometries(const Project &project, QVector<SphereGeometry> *spheres, QString *errorMessage) const;
    bool loadGeometryObjects(const Project &project, std::vector<GeometryObject> &geometries, QString *errorMessage) const;

private:
    QString geometryDirectory(const Project &project) const;
    QString nextBoxName(const QString &geometryDirPath) const;
    QString nextCylinderName(const QString &geometryDirPath) const;
    QString nextSphereName(const QString &geometryDirPath) const;
    QString nextBooleanName(const QString &geometryDirPath, GeometryBooleanOperationType operationType) const;
    bool writeBoxFile(const BoxGeometry &box, QString *errorMessage) const;
    bool readBoxFile(const QString &filePath, BoxGeometry *box, QString *errorMessage) const;
    bool writeCylinderFile(const CylinderGeometry &cylinder, QString *errorMessage) const;
    bool readCylinderFile(const QString &filePath, CylinderGeometry *cylinder, QString *errorMessage) const;
    bool writeSphereFile(const SphereGeometry &sphere, QString *errorMessage) const;
    bool readSphereFile(const QString &filePath, SphereGeometry *sphere, QString *errorMessage) const;
    bool writeBooleanGeometryFile(
        const Project &project,
        const GeometryObject &geometry,
        const GeometryObject &leftGeometry,
        const GeometryObject &rightGeometry,
        GeometryBooleanOperationType operationType,
        QString *errorMessage
    ) const;
    bool writeImportedStepGeometryFile(
        const Project &project,
        const GeometryObject &geometry,
        const QString &sourceStepFile,
        QString *errorMessage
    ) const;
};
