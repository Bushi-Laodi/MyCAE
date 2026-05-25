#include "geometry/FaceReferenceUtils.h"

#include <algorithm>

namespace FaceReferenceUtils
{
QString normalizedFaceGroupName(const QString &name)
{
    const QString trimmed = name.trimmed();
    return trimmed.isEmpty() ? FaceGroups::defaultName() : trimmed;
}

namespace
{
FaceReference faceReferenceFromPick(const PickSelection &selection)
{
    FaceReference reference;
    reference.faceIndex = selection.topologyIndex;
    reference.pickedPoint = {selection.worldX, selection.worldY, selection.worldZ};
    reference.center = {selection.centerX, selection.centerY, selection.centerZ};
    reference.normal = {selection.normalX, selection.normalY, selection.normalZ};
    reference.area = selection.area;
    return reference;
}
}

std::vector<FaceReference> normalizedReferences(const std::vector<PickSelection> &selections)
{
    std::vector<FaceReference> references;
    for (const PickSelection &selection : selections) {
        if (!selection.isValid()) {
            continue;
        }

        auto existing = std::find_if(references.begin(), references.end(), [&selection](const FaceReference &candidate) {
            return candidate.faceIndex == selection.topologyIndex;
        });
        if (existing == references.end()) {
            references.push_back(faceReferenceFromPick(selection));
        }
    }

    std::sort(references.begin(), references.end(), [](const FaceReference &lhs, const FaceReference &rhs) {
        return lhs.faceIndex < rhs.faceIndex;
    });
    return references;
}

void syncFaceIndices(FaceGroup &faceGroup)
{
    faceGroup.faceIndices.clear();
    for (const FaceReference &reference : faceGroup.faceReferences) {
        if (reference.faceIndex > 0) {
            faceGroup.faceIndices.push_back(reference.faceIndex);
        }
    }
    std::sort(faceGroup.faceIndices.begin(), faceGroup.faceIndices.end());
    faceGroup.faceIndices.erase(
        std::unique(faceGroup.faceIndices.begin(), faceGroup.faceIndices.end()),
        faceGroup.faceIndices.end()
    );
}

void ensureReferencesFromLegacyIndices(FaceGroup &faceGroup)
{
    if (!faceGroup.faceReferences.empty()) {
        syncFaceIndices(faceGroup);
        return;
    }

    std::sort(faceGroup.faceIndices.begin(), faceGroup.faceIndices.end());
    faceGroup.faceIndices.erase(
        std::unique(faceGroup.faceIndices.begin(), faceGroup.faceIndices.end()),
        faceGroup.faceIndices.end()
    );
    for (const int faceIndex : faceGroup.faceIndices) {
        if (faceIndex <= 0) {
            continue;
        }
        FaceReference reference;
        reference.faceIndex = faceIndex;
        faceGroup.faceReferences.push_back(reference);
    }
}

void replaceReferences(FaceGroup &faceGroup, std::vector<FaceReference> references)
{
    faceGroup.faceReferences = std::move(references);
    syncFaceIndices(faceGroup);
}

bool appendReferences(FaceGroup &faceGroup, const std::vector<FaceReference> &references)
{
    ensureReferencesFromLegacyIndices(faceGroup);
    bool changed = false;
    for (const FaceReference &reference : references) {
        auto existing = std::find_if(
            faceGroup.faceReferences.begin(),
            faceGroup.faceReferences.end(),
            [&reference](const FaceReference &candidate) {
                return candidate.faceIndex == reference.faceIndex;
            }
        );
        if (existing == faceGroup.faceReferences.end()) {
            faceGroup.faceReferences.push_back(reference);
            changed = true;
        } else {
            *existing = reference;
        }
    }
    std::sort(faceGroup.faceReferences.begin(), faceGroup.faceReferences.end(), [](const FaceReference &lhs, const FaceReference &rhs) {
        return lhs.faceIndex < rhs.faceIndex;
    });
    syncFaceIndices(faceGroup);
    return changed;
}

int removeReferences(FaceGroup &faceGroup, const std::vector<FaceReference> &references)
{
    ensureReferencesFromLegacyIndices(faceGroup);
    int removedCount = 0;
    for (const FaceReference &reference : references) {
        const auto oldSize = faceGroup.faceReferences.size();
        faceGroup.faceReferences.erase(
            std::remove_if(
                faceGroup.faceReferences.begin(),
                faceGroup.faceReferences.end(),
                [&reference](const FaceReference &candidate) {
                    return candidate.faceIndex == reference.faceIndex;
                }
            ),
            faceGroup.faceReferences.end()
        );
        removedCount += static_cast<int>(oldSize - faceGroup.faceReferences.size());
    }
    syncFaceIndices(faceGroup);
    return removedCount;
}
}
