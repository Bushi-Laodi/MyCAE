#include "project/ResultRepository.h"

void ResultRepository::clear()
{
    m_results.clear();
}

std::vector<ResultObject> &ResultRepository::results()
{
    return m_results;
}

const std::vector<ResultObject> &ResultRepository::results() const
{
    return m_results;
}

ResultObject *ResultRepository::findResultById(const QString &id)
{
    for (ResultObject &result : m_results) {
        if (result.id == id) {
            return &result;
        }
    }
    return nullptr;
}

const ResultObject *ResultRepository::findResultById(const QString &id) const
{
    for (const ResultObject &result : m_results) {
        if (result.id == id) {
            return &result;
        }
    }
    return nullptr;
}
