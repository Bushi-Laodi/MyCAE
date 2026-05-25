#include "diagnostics/DiagnosticMessage.h"

QString diagnosticSeverityName(DiagnosticSeverity severity)
{
    switch (severity) {
    case DiagnosticSeverity::Info:
        return "Info";
    case DiagnosticSeverity::Warning:
        return "Warning";
    case DiagnosticSeverity::Error:
        return "Error";
    }
    return "Unknown";
}

QString diagnosticCategoryName(DiagnosticCategory category)
{
    switch (category) {
    case DiagnosticCategory::Environment:
        return "Environment";
    case DiagnosticCategory::Input:
        return "Input";
    case DiagnosticCategory::Mesh:
        return "Mesh";
    case DiagnosticCategory::Solver:
        return "Solver";
    case DiagnosticCategory::Result:
        return "Result";
    case DiagnosticCategory::UI:
        return "UI";
    case DiagnosticCategory::Project:
        return "Project";
    case DiagnosticCategory::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

