#pragma once

#include "diagnostics/DiagnosticMessage.h"

#include <QString>
#include <QVector>

class DiagnosticCollector
{
public:
    void clear();

    bool addFromLogMessage(const QString &message);
    void addDiagnostic(const DiagnosticMessage &diagnostic);
    const QVector<DiagnosticMessage> &diagnostics() const;

private:
    QVector<DiagnosticMessage> m_diagnostics;
};

