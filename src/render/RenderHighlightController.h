#pragma once

#include "geometry/FaceGroup.h"

#include <QStringList>

class RenderView;

struct RenderHighlightResult
{
    bool success = false;
    QStringList logMessages;
};

class RenderHighlightController
{
public:
    RenderHighlightResult clear(RenderView *renderView) const;
    RenderHighlightResult highlightFaceGroup(const FaceGroup &faceGroup, RenderView *renderView) const;
    RenderHighlightResult highlightBoundaryConditionFaceGroup(const FaceGroup &faceGroup, RenderView *renderView) const;
    RenderHighlightResult highlightLoadFaceGroup(const FaceGroup &faceGroup, RenderView *renderView) const;
};
