#include "picking/PickController.h"

#include "geometry/FaceGroupService.h"
#include "project/ProjectModel.h"
#include "ui/RenderView.h"

#include <algorithm>

namespace
{
bool toggleSelection(std::vector<PickSelection> &selections, const PickSelection &selection)
{
    auto it = std::find_if(selections.begin(), selections.end(), [&selection](const PickSelection &candidate) {
        return candidate.topologyIndex == selection.topologyIndex;
    });
    if (it != selections.end()) {
        selections.erase(it);
        return false;
    }

    selections.push_back(selection);
    std::sort(selections.begin(), selections.end(), [](const PickSelection &lhs, const PickSelection &rhs) {
        return lhs.topologyIndex < rhs.topologyIndex;
    });
    return true;
}

std::vector<int> faceIndicesFromSelections(const std::vector<PickSelection> &selections)
{
    std::vector<int> faceIndices;
    for (const PickSelection &selection : selections) {
        if (selection.topologyIndex > 0) {
            faceIndices.push_back(selection.topologyIndex);
        }
    }
    return faceIndices;
}
}

PickMode PickController::mode() const
{
    return m_mode;
}

const PickSelection &PickController::selection() const
{
    return m_selection;
}

const QString &PickController::geometryName() const
{
    return m_geometryName;
}

const std::vector<int> &PickController::selectedFaceIndices() const
{
    return m_faceIndices;
}

const std::vector<PickSelection> &PickController::selections() const
{
    return m_selections;
}

bool PickController::hasSelection() const
{
    return !m_geometryName.isEmpty() && !m_selections.empty();
}

void PickController::setTargetGeometry(const QString &geometryName)
{
    m_targetGeometryName = geometryName.trimmed();
}

const QString &PickController::targetGeometry() const
{
    return m_targetGeometryName;
}

PickControllerResult PickController::setMode(PickMode mode, RenderView *renderView)
{
    PickControllerResult result;
    m_mode = mode;
    if (renderView) {
        renderView->setPickMode(mode);
    }

    result.success = true;
    result.logMessages.append("Pick mode: " + pickModeName(mode));
    return result;
}

PickControllerResult PickController::clear(RenderView *renderView, bool clearRenderHighlight)
{
    PickControllerResult result;
    m_selection = {};
    m_geometryName.clear();
    m_faceIndices.clear();
    m_selections.clear();
    if (renderView && clearRenderHighlight) {
        renderView->clearHighlight();
    }

    result.success = true;
    result.logMessages.append("Pick selection cleared.");
    return result;
}

PickControllerResult PickController::acceptSelection(const PickSelection &selection, RenderView *renderView)
{
    PickControllerResult result;
    if (m_mode == PickMode::None) {
        return result;
    }
    if (selection.mode != m_mode || !selection.isValid()) {
        result.logMessages.append("Pick ignored: selection does not match the current pick mode.");
        return result;
    }
    if (!m_targetGeometryName.isEmpty() && selection.geometryName != m_targetGeometryName) {
        result.logMessages.append("Pick ignored: current pick target is " + m_targetGeometryName + ".");
        return result;
    }

    m_selection = selection;
    if (m_geometryName != selection.geometryName) {
        m_geometryName = selection.geometryName;
        m_faceIndices.clear();
        m_selections.clear();
    }
    const bool selected = toggleSelection(m_selections, selection);
    m_faceIndices = faceIndicesFromSelections(m_selections);
    if (renderView && selection.mode == PickMode::Face) {
        renderView->highlightFaceIndices(m_faceIndices);
    }

    result.success = true;
    result.geometryName = m_geometryName;
    result.selectedFaceCount = static_cast<int>(m_faceIndices.size());
    result.logMessages.append(
        QString("%1 %2 %3 on geometry %4. Current pick set: %5 face(s).")
            .arg(selected ? "Picked" : "Unpicked")
            .arg(pickModeName(selection.mode))
            .arg(selection.topologyIndex)
            .arg(selection.geometryName)
            .arg(m_faceIndices.size())
    );
    return result;
}

PickControllerResult PickController::createFaceGroupFromCurrentPick(
    ProjectModel &projectModel,
    const QString &faceGroupName
)
{
    PickControllerResult result;
    if (!projectModel.hasProject()) {
        result.logMessages.append("Create face group failed: create or open a project first.");
        return result;
    }
    if (!hasSelection()) {
        result.logMessages.append("Create face group failed: pick one or more faces first.");
        return result;
    }
    if (!projectModel.findGeometryByName(m_geometryName)) {
        result.logMessages.append("Create face group failed: picked geometry is no longer available.");
        return result;
    }

    const FaceGroupServiceResult serviceResult = FaceGroupService::createOrReplacePickedFaces(
        projectModel,
        m_geometryName,
        faceGroupName,
        m_selections
    );
    result.logMessages.append(serviceResult.logMessages);
    if (!serviceResult.success) {
        return result;
    }

    result.success = true;
    result.faceGroupChanged = true;
    result.faceGroupId = serviceResult.faceGroupId;
    result.geometryName = m_geometryName;
    result.selectedFaceCount = static_cast<int>(m_faceIndices.size());
    return result;
}
