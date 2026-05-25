#pragma once

#include "solver/export/SolverCaseWriterResult.h"
#include "solver/plugin/SolverCaseContext.h"
#include "solver/plugin/SolverPluginDescriptor.h"
#include "solver/plugin/SolverResultReadResult.h"
#include "solver/plugin/SolverRunResult.h"

#include <QString>

class SolverPlugin
{
public:
    virtual ~SolverPlugin() = default;

    virtual SolverPluginDescriptor descriptor() const = 0;

    QString id() const
    {
        return descriptor().id;
    }

    QString name() const
    {
        return descriptor().name;
    }

    virtual SolverCaseWriterResult exportCase(const SolverCaseContext &context) const = 0;
    virtual SolverRunResult runCase(const SolverCaseContext &context) const = 0;
    virtual SolverResultReadResult readResult(const SolverCaseContext &context) const = 0;
};
