#pragma once

#include "result/ResultObject.h"

#include <QStringList>

class ProjectModel;
class RenderView;

struct ResultDisplayResult
{
    bool success = false;
    QStringList logMessages;
};

class ResultDisplayController
{
public:
    ResultDisplayResult displayResult(
        const ProjectModel &projectModel,
        const ResultObject &resultObject,
        RenderView *renderView
    ) const;
};
