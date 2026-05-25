#include "solver/plugin/SolverPluginDescriptorFormatter.h"

namespace
{
QString capabilityText(const SolverCapabilities &capabilities)
{
    QStringList items;
    if (capabilities.canExportCase) {
        items.append("export");
    }
    if (capabilities.canRunCase) {
        items.append("run");
    }
    if (capabilities.canReadResult) {
        items.append("read-result");
    }
    return items.isEmpty() ? QString("none") : items.join(", ");
}
}

QString SolverPluginDescriptorFormatter::statusText(SolverPluginStatus status)
{
    switch (status) {
    case SolverPluginStatus::Ready:
        return "ready";
    case SolverPluginStatus::Reserved:
        return "reserved";
    case SolverPluginStatus::Unavailable:
        return "unavailable";
    }
    return "unavailable";
}

QString SolverPluginDescriptorFormatter::menuText(const SolverPluginDescriptor &descriptor)
{
    if (descriptor.status == SolverPluginStatus::Reserved) {
        return "Run " + descriptor.name + " (reserved)";
    }
    if (descriptor.status == SolverPluginStatus::Unavailable) {
        return "Run " + descriptor.name + " (unavailable)";
    }
    return "Run " + descriptor.name;
}

QString SolverPluginDescriptorFormatter::statusTip(const SolverPluginDescriptor &descriptor)
{
    QStringList parts;
    parts.append(descriptor.description.trimmed().isEmpty()
        ? QString("Solver plugin: %1").arg(descriptor.name)
        : descriptor.description.trimmed());
    parts.append("family=" + descriptor.solverFamily);
    parts.append("status=" + statusText(descriptor.status));
    parts.append("capabilities=" + capabilityText(descriptor.capabilities));
    if (!descriptor.capabilities.analysisTypes.isEmpty()) {
        parts.append("analysis=" + descriptor.capabilities.analysisTypes.join(", "));
    }
    return parts.join(" | ");
}

QString SolverPluginDescriptorFormatter::registrationLog(
    const SolverPluginDescriptor &descriptor,
    const QString &source
)
{
    return QString("Registered solver plugin: %1 (%2), source=%3, status=%4, capabilities=%5.")
        .arg(descriptor.name)
        .arg(descriptor.id)
        .arg(source)
        .arg(statusText(descriptor.status))
        .arg(capabilityText(descriptor.capabilities));
}
