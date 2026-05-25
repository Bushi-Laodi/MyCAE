#pragma once

#include "solver/plugin/SolverPluginDescriptor.h"

#include <QString>

class SolverPluginDescriptorFormatter
{
public:
    static QString statusText(SolverPluginStatus status);
    static QString menuText(const SolverPluginDescriptor &descriptor);
    static QString statusTip(const SolverPluginDescriptor &descriptor);
    static QString registrationLog(const SolverPluginDescriptor &descriptor, const QString &source);
};
