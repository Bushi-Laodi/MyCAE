#include "render/RenderHighlightController.h"

#include "ui/RenderView.h"

namespace
{
VtkHighlightStyle selectedFaceGroupStyle()
{
    return {};
}

VtkHighlightStyle boundaryConditionStyle()
{
    VtkHighlightStyle style;
    style.red = 0.02;
    style.green = 0.72;
    style.blue = 0.92;
    style.edgeRed = 0.67;
    style.edgeGreen = 0.95;
    style.edgeBlue = 1.0;
    style.opacity = 0.88;
    style.lineWidth = 3.4;
    return style;
}

VtkHighlightStyle loadStyle()
{
    VtkHighlightStyle style;
    style.red = 1.0;
    style.green = 0.24;
    style.blue = 0.12;
    style.edgeRed = 1.0;
    style.edgeGreen = 0.74;
    style.edgeBlue = 0.45;
    style.opacity = 0.90;
    style.lineWidth = 3.6;
    return style;
}

RenderHighlightResult highlightFaceGroupWithStyle(
    const FaceGroup &faceGroup,
    RenderView *renderView,
    const VtkHighlightStyle &style,
    const QString &logPrefix
)
{
    RenderHighlightResult result;
    if (!renderView) {
        result.logMessages.append(logPrefix + " skipped: render view is not available.");
        return result;
    }
    if (faceGroup.faceIndices.empty() && faceGroup.faceReferences.empty()) {
        result.logMessages.append("Face group has no picked face indices: " + faceGroup.id);
        return result;
    }

    renderView->highlightFaceGroup(faceGroup, style);
    result.success = true;
    result.logMessages.append(logPrefix + ": " + faceGroup.id);
    return result;
}
}

RenderHighlightResult RenderHighlightController::clear(RenderView *renderView) const
{
    RenderHighlightResult result;
    if (!renderView) {
        result.logMessages.append("Render highlight skipped: render view is not available.");
        return result;
    }

    renderView->clearHighlight();
    result.success = true;
    return result;
}

RenderHighlightResult RenderHighlightController::highlightFaceGroup(
    const FaceGroup &faceGroup,
    RenderView *renderView
) const
{
    return highlightFaceGroupWithStyle(faceGroup, renderView, selectedFaceGroupStyle(), "Face group highlighted");
}

RenderHighlightResult RenderHighlightController::highlightBoundaryConditionFaceGroup(
    const FaceGroup &faceGroup,
    RenderView *renderView
) const
{
    return highlightFaceGroupWithStyle(faceGroup, renderView, boundaryConditionStyle(), "Boundary condition face group highlighted");
}

RenderHighlightResult RenderHighlightController::highlightLoadFaceGroup(
    const FaceGroup &faceGroup,
    RenderView *renderView
) const
{
    return highlightFaceGroupWithStyle(faceGroup, renderView, loadStyle(), "Load face group highlighted");
}
