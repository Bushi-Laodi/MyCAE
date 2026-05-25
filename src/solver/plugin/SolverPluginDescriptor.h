#pragma once

#include "solver/plugin/SolverCapabilities.h"

#include <QString>

enum class SolverPluginStatus
{
    Ready,
    Reserved,
    Unavailable
};

struct SolverPluginDescriptor
{
    QString id;
    QString name;
    QString vendor;
    QString version;
    QString solverFamily;
    QString description;
    SolverPluginStatus status = SolverPluginStatus::Unavailable;
    SolverCapabilities capabilities;

    bool isUsable() const
    {
        return status == SolverPluginStatus::Ready;
    }
};
