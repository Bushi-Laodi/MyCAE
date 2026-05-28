#include "result/ResultDisplayCache.h"

#include "mesh/MeshToVtkConverter.h"
#include "result/ResultExtremaCalculator.h"
#include "result/ResultObject.h"

#include <QDateTime>
#include <QFileInfo>

namespace
{
QString timestampToken(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return {};
    }
    const QFileInfo info(filePath);
    return info.exists()
        ? QString::number(info.lastModified().toMSecsSinceEpoch()) + ":" + QString::number(info.size())
        : QString();
}
}

ResultDisplayCache &ResultDisplayCache::instance()
{
    static auto *cache = new ResultDisplayCache();
    return *cache;
}

ResultDisplayCacheData ResultDisplayCache::loadData(
    const ProjectModel &projectModel,
    const ResultObject &resultObject
)
{
    const QString cacheKey = keyFor(resultObject);
    if (m_loaded && (m_dataKey == cacheKey || m_dataKey.startsWith(cacheKey + "|"))) {
        return {m_loaded, true};
    }

    auto loaded = std::make_shared<ResultDataLoadResult>(
        ResultDataLoader().loadCalculiXResult(projectModel, resultObject)
    );
    const QString resolvedKey = cacheKey
        + "|meshFile=" + loaded->meshFilePath
        + "|meshStamp=" + timestampToken(loaded->meshFilePath)
        + "|datFile=" + loaded->datFilePath
        + "|datStamp=" + timestampToken(loaded->datFilePath);

    if (m_loaded && m_dataKey == resolvedKey) {
        return {m_loaded, true};
    }

    m_dataKey = resolvedKey;
    m_loaded = loaded;
    m_gridKey.clear();
    m_gridResult = {};
    m_overlayKey.clear();
    m_overlayGrid = nullptr;
    m_overlayWarnings.clear();
    m_extremaByKey.clear();
    return {m_loaded, false};
}

ResultDisplayCacheGrid ResultDisplayCache::buildGrid(
    const ResultDataLoadResult &loaded,
    const QString &resultKey,
    double deformationScale
)
{
    const QString gridKey = m_dataKey + "|result=" + resultKey;
    if (m_gridResult.success && m_gridKey == gridKey && m_gridScale == deformationScale) {
        return {m_gridResult, true};
    }

    m_gridResult = CalculiXResultGridBuilder().buildResultGrid(
        loaded.meshData,
        loaded.datResult,
        CalculiXResultFields::DisplacementMagnitude,
        deformationScale
    );
    m_gridKey = gridKey;
    m_gridScale = deformationScale;
    return {m_gridResult, false};
}

ResultDisplayCacheOverlay ResultDisplayCache::buildOverlay(
    const ResultDataLoadResult &loaded,
    const QString &resultKey
)
{
    const QString overlayKey = m_dataKey + "|overlay=" + resultKey;
    if (m_overlayGrid && m_overlayKey == overlayKey) {
        return {m_overlayGrid, m_overlayWarnings, true};
    }

    QString overlayError;
    m_overlayGrid = MeshToVtkConverter::toUnstructuredGrid(loaded.meshData, &overlayError);
    m_overlayKey = overlayKey;
    m_overlayWarnings.clear();
    if (!m_overlayGrid) {
        m_overlayWarnings.append("Cannot build undeformed overlay: " + overlayError);
    }
    return {m_overlayGrid, m_overlayWarnings, false};
}

ResultDisplayCacheExtrema ResultDisplayCache::calculateExtrema(
    const ResultDataLoadResult &loaded,
    const QString &resultKey,
    const QString &fieldName,
    double deformationScale
)
{
    const QString extremaKey = m_dataKey
        + "|extrema=" + resultKey
        + "|field=" + fieldName
        + "|scale=" + QString::number(deformationScale, 'g', 16);
    const auto existing = m_extremaByKey.constFind(extremaKey);
    if (existing != m_extremaByKey.constEnd()) {
        return {*existing, true};
    }

    const ResultExtrema extrema = ResultExtremaCalculator().calculate(
        loaded.meshData,
        loaded.datResult,
        fieldName,
        deformationScale
    );
    m_extremaByKey.insert(extremaKey, extrema);
    return {extrema, false};
}

void ResultDisplayCache::clear()
{
    m_dataKey.clear();
    m_loaded.reset();
    m_gridKey.clear();
    m_gridScale = 0.0;
    m_gridResult = {};
    m_overlayKey.clear();
    m_overlayGrid = nullptr;
    m_overlayWarnings.clear();
    m_extremaByKey.clear();
}

QString ResultDisplayCache::keyFor(const ResultObject &resultObject) const
{
    return resultObject.id
        + "|mesh=" + resultObject.meshName
        + "|case=" + resultObject.casePath
        + "|dat=" + resultObject.datFile
        + "|frd=" + resultObject.frdFile
        + "|created=" + resultObject.createdAt;
}
