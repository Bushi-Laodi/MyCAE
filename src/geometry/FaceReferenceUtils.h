#pragma once

#include "geometry/FaceGroup.h"
#include "picking/PickSelection.h"

#include <QString>

#include <vector>

namespace FaceReferenceUtils
{
QString normalizedFaceGroupName(const QString &name);
std::vector<FaceReference> normalizedReferences(const std::vector<PickSelection> &selections);
void syncFaceIndices(FaceGroup &faceGroup);
void ensureReferencesFromLegacyIndices(FaceGroup &faceGroup);
void replaceReferences(FaceGroup &faceGroup, std::vector<FaceReference> references);
bool appendReferences(FaceGroup &faceGroup, const std::vector<FaceReference> &references);
int removeReferences(FaceGroup &faceGroup, const std::vector<FaceReference> &references);
}
