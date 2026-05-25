#pragma once

#include "result/ResultObject.h"

#include <vector>

class ResultRepository
{
public:
    void clear();

    std::vector<ResultObject> &results();
    const std::vector<ResultObject> &results() const;
    ResultObject *findResultById(const QString &id);
    const ResultObject *findResultById(const QString &id) const;

private:
    std::vector<ResultObject> m_results;
};
