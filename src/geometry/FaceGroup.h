#pragma once

#include <QString>

struct FaceGroup
{
    QString id;
    QString name;
    QString geometryName;
    QString role = "Default";
};

namespace FaceGroups
{
inline QString defaultName()
{
    return "Default";
}

inline QString makeId(const QString &geometryName, const QString &faceGroupName)
{
    const QString geometry = geometryName.trimmed();
    const QString name = faceGroupName.trimmed();
    if (geometry.isEmpty() || name.contains('.')) {
        return name;
    }
    return geometry + "." + name;
}

inline QString nameFromId(const QString &faceGroupId)
{
    const QString id = faceGroupId.trimmed();
    const int separatorIndex = id.lastIndexOf('.');
    return separatorIndex >= 0 ? id.mid(separatorIndex + 1) : id;
}

inline QString displayName(const FaceGroup &faceGroup)
{
    if (!faceGroup.geometryName.trimmed().isEmpty() && !faceGroup.name.trimmed().isEmpty()) {
        return faceGroup.geometryName.trimmed() + "." + faceGroup.name.trimmed();
    }
    return faceGroup.id;
}

inline FaceGroup makeDefault(const QString &geometryName)
{
    FaceGroup faceGroup;
    faceGroup.name = defaultName();
    faceGroup.geometryName = geometryName.trimmed();
    faceGroup.id = makeId(faceGroup.geometryName, faceGroup.name);
    faceGroup.role = defaultName();
    return faceGroup;
}
}
