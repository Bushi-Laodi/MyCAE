#pragma once

#include "geometry/BoxGeometry.h"
#include "project/Project.h"

#include <QVector>

class QString;

class GeometryManager
{
public:
    bool createDefaultBox(const Project &project, BoxGeometry *box, QString *errorMessage) const;
    bool createBox(const Project &project, const BoxGeometry &parameters, BoxGeometry *box, QString *errorMessage) const;
    bool loadBoxGeometries(const Project &project, QVector<BoxGeometry> *boxes, QString *errorMessage) const;

private:
    QString geometryDirectory(const Project &project) const;
    QString nextBoxName(const QString &geometryDirPath) const;
    bool writeBoxFile(const BoxGeometry &box, QString *errorMessage) const;
    bool readBoxFile(const QString &filePath, BoxGeometry *box, QString *errorMessage) const;
};
