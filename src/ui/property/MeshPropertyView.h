#pragma once

class QWidget;
struct MeshObject;

#include <functional>

struct MeshPropertyActions
{
    std::function<void()> highlightQualityIssues;
    std::function<void()> clearHighlight;
};

class MeshPropertyView
{
public:
    static void populate(
        QWidget *parent,
        const MeshObject &meshObject,
        const MeshPropertyActions &actions = {}
    );
};
