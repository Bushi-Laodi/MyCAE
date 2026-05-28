#pragma once

class QWidget;
struct MeshObject;

class MeshPropertyView
{
public:
    static void populate(QWidget *parent, const MeshObject &meshObject);
};
