#pragma once

#include "geometry/FaceGroup.h"
#include "picking/PickMode.h"
#include "picking/PickSelection.h"

#include <QString>
#include <QStringList>

#include <vector>

class ProjectModel;
class RenderView;

struct PickControllerResult
{
    bool success = false;
    bool faceGroupChanged = false;
    QString faceGroupId;
    QString geometryName;
    int selectedFaceCount = 0;
    QStringList logMessages;
};

class PickController
{
public:
    PickMode mode() const;
    const PickSelection &selection() const;
    const QString &geometryName() const;
    const std::vector<int> &selectedFaceIndices() const;
    const std::vector<PickSelection> &selections() const;
    bool hasSelection() const;
    void setTargetGeometry(const QString &geometryName);
    const QString &targetGeometry() const;

    PickControllerResult setMode(PickMode mode, RenderView *renderView);
    PickControllerResult clear(RenderView *renderView);
    PickControllerResult acceptSelection(const PickSelection &selection, RenderView *renderView);
    PickControllerResult createFaceGroupFromCurrentPick(ProjectModel &projectModel, const QString &faceGroupName);

private:
    PickMode m_mode = PickMode::None;
    PickSelection m_selection;
    QString m_geometryName;
    std::vector<int> m_faceIndices;
    std::vector<PickSelection> m_selections;
    QString m_targetGeometryName;
};
