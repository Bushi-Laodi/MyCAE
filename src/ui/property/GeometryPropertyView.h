#pragma once

class QWidget;
struct GeometryObject;

class GeometryPropertyView
{
public:
    static void populate(QWidget *parent, const GeometryObject &geometry);
};
