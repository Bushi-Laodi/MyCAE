#pragma once

#include <QString>

class TopoDS_Shape;

class OCCShapeIO
{
public:
    bool saveBREP(const TopoDS_Shape &shape, const QString &filePath, QString *errorMessage = nullptr) const;
    bool loadBREP(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage = nullptr) const;
    bool saveSTEP(const TopoDS_Shape &shape, const QString &filePath, QString *errorMessage = nullptr) const;
    bool loadSTEP(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage = nullptr) const;
};
