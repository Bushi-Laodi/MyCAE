#pragma once

#include "project/Project.h"
#include "result/ResultObject.h"

#include <QString>

#include <vector>

class ResultManager
{
public:
    static QString relativeResultsFilePath();

    bool exists(const Project &project) const;
    bool load(const Project &project, std::vector<ResultObject> &results, QString *errorMessage = nullptr) const;
    bool save(const Project &project, const std::vector<ResultObject> &results, QString *errorMessage = nullptr) const;
};
