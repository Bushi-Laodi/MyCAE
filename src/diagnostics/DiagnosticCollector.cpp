#include "diagnostics/DiagnosticCollector.h"

namespace
{
bool containsAny(const QString &text, const QStringList &patterns)
{
    for (const QString &pattern : patterns) {
        if (text.contains(pattern)) {
            return true;
        }
    }
    return false;
}

DiagnosticSeverity severityForMessage(const QString &lower)
{
    if (containsAny(lower, {"[fail]", " fail", "failed", "error", "cannot ", "does not exist", "not found", "invalid"})) {
        return DiagnosticSeverity::Error;
    }
    if (containsAny(lower, {" warning", "warning:", "skipped", "incomplete", "missing"})) {
        return DiagnosticSeverity::Warning;
    }
    return DiagnosticSeverity::Info;
}

DiagnosticCategory categoryForMessage(const QString &lower)
{
    if (containsAny(lower, {"calculix path", "gmsh path", "environment", "executable", "dll", "ccx.exe", "gmsh.exe"})) {
        return DiagnosticCategory::Environment;
    }
    if (containsAny(lower, {"mesh", "msh", "tetra", "node count", "tetra count", "surface triangle"})) {
        return DiagnosticCategory::Mesh;
    }
    if (containsAny(lower, {"result", ".dat", ".frd", "displacement", "stress", "von mises", "coverage"})) {
        return DiagnosticCategory::Result;
    }
    if (containsAny(lower, {"solver", "solved", "calculix", "openfoam", "analysis", ".sta", ".log", "converge", "ccx"})) {
        return DiagnosticCategory::Solver;
    }
    if (containsAny(lower, {"material", "boundary", "load", "case data", "no enabled", "no material"})) {
        return DiagnosticCategory::Input;
    }
    if (containsAny(lower, {"project", "open project", "new project", "save simulation case"})) {
        return DiagnosticCategory::Project;
    }
    if (containsAny(lower, {"selection", "selected", "canceled", "pick", "display skipped"})) {
        return DiagnosticCategory::UI;
    }
    return DiagnosticCategory::Unknown;
}

QString suggestedFixFor(DiagnosticCategory category, DiagnosticSeverity severity)
{
    if (severity == DiagnosticSeverity::Info) {
        return {};
    }

    switch (category) {
    case DiagnosticCategory::Environment:
        return "Check executable paths, PATH/DLL runtime directories, and Debug/Release library consistency.";
    case DiagnosticCategory::Input:
        return "Check that the simulation case has material, boundary condition, load, and valid target face groups.";
    case DiagnosticCategory::Mesh:
        return "Regenerate the mesh, verify the .msh file exists, and confirm node/tetra counts are nonzero.";
    case DiagnosticCategory::Solver:
        return "Open the solver case directory and inspect .log, .sta, .dat, and the exported input deck.";
    case DiagnosticCategory::Result:
        return "Rerun the solver with result requests enabled and verify .dat/.frd/.sta files were generated.";
    case DiagnosticCategory::UI:
        return "Select the required project tree item before running this command.";
    case DiagnosticCategory::Project:
        return "Check the project root, project.json, and write permissions under the project directory.";
    case DiagnosticCategory::Unknown:
        return "Check the log context around this message for the failing operation.";
    }
    return {};
}

bool shouldCollect(const QString &lower)
{
    return containsAny(lower, {
        "failed",
        "[fail]",
        "warning",
        "error",
        "cannot",
        "does not exist",
        "not found",
        "invalid",
        "missing",
        "incomplete",
        "skipped",
        "no result",
        "no enabled",
        "no material",
        "no load",
        "no boundary"
    });
}
}

void DiagnosticCollector::clear()
{
    m_diagnostics.clear();
}

bool DiagnosticCollector::addFromLogMessage(const QString &message)
{
    const QString trimmed = message.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    const QString lower = trimmed.toLower();
    if (!shouldCollect(lower)) {
        return false;
    }

    DiagnosticMessage diagnostic;
    diagnostic.severity = severityForMessage(lower);
    diagnostic.category = categoryForMessage(lower);
    diagnostic.message = trimmed;
    diagnostic.suggestedFix = suggestedFixFor(diagnostic.category, diagnostic.severity);
    addDiagnostic(diagnostic);
    return true;
}

void DiagnosticCollector::addDiagnostic(const DiagnosticMessage &diagnostic)
{
    for (const DiagnosticMessage &existing : m_diagnostics) {
        if (existing.severity == diagnostic.severity
                && existing.category == diagnostic.category
                && existing.message == diagnostic.message) {
            return;
        }
    }
    m_diagnostics.append(diagnostic);
}

const QVector<DiagnosticMessage> &DiagnosticCollector::diagnostics() const
{
    return m_diagnostics;
}
