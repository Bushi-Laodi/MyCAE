#include "render/RenderHighlightController.h"

#include "ui/RenderView.h"

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
    RenderHighlightResult result;
    if (!renderView) {
        result.logMessages.append("Face group highlight skipped: render view is not available.");
        return result;
    }
    if (faceGroup.faceIndices.empty() && faceGroup.faceReferences.empty()) {
        result.logMessages.append("Face group has no picked face indices: " + faceGroup.id);
        return result;
    }

    renderView->highlightFaceGroup(faceGroup);
    result.success = true;
    result.logMessages.append("Face group highlighted: " + faceGroup.id);
    return result;
}
