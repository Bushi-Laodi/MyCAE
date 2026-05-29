#include "diagnostics/DiagnosticCollector.h"

#include <QStringList>

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
    if (containsAny(lower, {
            "[fail]",
            " fail",
            "failed",
            "error",
            "cannot ",
            "does not exist",
            "not found",
            "invalid",
            "degenerate",
            "zero volume",
            "negative jacobian",
            "stale",
            "failed:",
            "失败",
            "不存在",
            "无效",
            "错误",
            "退化"
        })) {
        return DiagnosticSeverity::Error;
    }
    if (containsAny(lower, {
            " warning",
            "warning:",
            "skipped",
            "incomplete",
            "missing",
            "diagnostic hint",
            "mesh quality",
            "aspect ratio",
            "high aspect",
            "警告",
            "缺失",
            "需要检查",
            "长宽比"
        })) {
        return DiagnosticSeverity::Warning;
    }
    return DiagnosticSeverity::Info;
}

DiagnosticCategory categoryForMessage(const QString &lower)
{
    if (containsAny(lower, {"calculix path", "gmsh path", "environment", "executable", "dll", "ccx.exe", "gmsh.exe"})) {
        return DiagnosticCategory::Environment;
    }
    if (containsAny(lower, {
            "mesh",
            "msh",
            "tetra",
            "node count",
            "tetra count",
            "surface triangle",
            "mesh quality",
            "aspect ratio",
            "degenerate",
            "zero volume",
            "negative jacobian",
            "网格",
            "四面体",
            "节点",
            "长宽比",
            "退化"
        })) {
        return DiagnosticCategory::Mesh;
    }
    if (containsAny(lower, {"result", ".dat", ".frd", "displacement", "stress", "von mises", "coverage", "结果"})) {
        return DiagnosticCategory::Result;
    }
    if (containsAny(lower, {
            "solver",
            "solved",
            "calculix",
            "openfoam",
            "analysis",
            ".sta",
            ".log",
            "converge",
            "ccx",
            "zero pivot",
            "singular",
            "cutbacks",
            "求解器",
            "求解"
        })) {
        return DiagnosticCategory::Solver;
    }
    if (containsAny(lower, {
            "material",
            "boundary",
            "load",
            "case data",
            "no enabled",
            "no material",
            "no load",
            "no boundary",
            "材料",
            "边界",
            "载荷",
            "工况",
            "面组"
        })) {
        return DiagnosticCategory::Input;
    }
    if (containsAny(lower, {"project", "open project", "new project", "save simulation case", "工程"})) {
        return DiagnosticCategory::Project;
    }
    if (containsAny(lower, {"selection", "selected", "canceled", "pick", "display skipped", "选择", "拾取"})) {
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
        return "Check material, boundary condition, load, and target face group definitions.";
    case DiagnosticCategory::Mesh:
        return "Regenerate the mesh, refine local mesh controls, verify node/tetra counts, and resolve degenerate or high-aspect elements before solving.";
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
        "mesh quality",
        "aspect ratio",
        "high aspect",
        "degenerate",
        "zero volume",
        "negative jacobian",
        "missing",
        "incomplete",
        "skipped",
        "no result",
        "no enabled",
        "no material",
        "no load",
        "no boundary",
        "diagnostic",
        "zero pivot",
        "singular",
        "too many cutbacks",
        "stale",
        "警告",
        "失败",
        "不存在",
        "无效",
        "错误",
        "缺失",
        "需要检查",
        "退化",
        "长宽比"
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
