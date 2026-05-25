#pragma once

#include <QString>

enum class DiagnosticSeverity
{
    Info,
    Warning,
    Error
};

enum class DiagnosticCategory
{
    Environment,
    Input,
    Mesh,
    Solver,
    Result,
    UI,
    Project,
    Unknown
};

struct DiagnosticMessage
{
    DiagnosticSeverity severity = DiagnosticSeverity::Info;
    DiagnosticCategory category = DiagnosticCategory::Unknown;
    QString message;
    QString suggestedFix;
};

QString diagnosticSeverityName(DiagnosticSeverity severity);
QString diagnosticCategoryName(DiagnosticCategory category);

