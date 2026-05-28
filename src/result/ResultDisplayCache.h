#pragma once

#include "result/ResultDataLoader.h"
#include "result/ResultExtrema.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <memory>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

class ProjectModel;
struct ResultObject;

struct ResultDisplayCacheData
{
    std::shared_ptr<ResultDataLoadResult> loaded;
    bool cacheHit = false;
};

struct ResultDisplayCacheGrid
{
    CalculiXResultGridBuildResult gridResult;
    bool cacheHit = false;
};

struct ResultDisplayCacheOverlay
{
    vtkSmartPointer<vtkUnstructuredGrid> grid;
    QStringList warnings;
    bool cacheHit = false;
};

struct ResultDisplayCacheExtrema
{
    ResultExtrema extrema;
    bool cacheHit = false;
};

class ResultDisplayCache
{
public:
    static ResultDisplayCache &instance();

    ResultDisplayCacheData loadData(const ProjectModel &projectModel, const ResultObject &resultObject);
    ResultDisplayCacheGrid buildGrid(
        const ResultDataLoadResult &loaded,
        const QString &resultKey,
        double deformationScale
    );
    ResultDisplayCacheOverlay buildOverlay(const ResultDataLoadResult &loaded, const QString &resultKey);
    ResultDisplayCacheExtrema calculateExtrema(
        const ResultDataLoadResult &loaded,
        const QString &resultKey,
        const QString &fieldName,
        double deformationScale
    );
    void clear();

private:
    QString keyFor(const ResultObject &resultObject) const;

    QString m_dataKey;
    std::shared_ptr<ResultDataLoadResult> m_loaded;
    QString m_gridKey;
    double m_gridScale = 0.0;
    CalculiXResultGridBuildResult m_gridResult;
    QString m_overlayKey;
    vtkSmartPointer<vtkUnstructuredGrid> m_overlayGrid;
    QStringList m_overlayWarnings;
    QHash<QString, ResultExtrema> m_extremaByKey;
};
