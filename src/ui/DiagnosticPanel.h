#pragma once

#include "diagnostics/DiagnosticMessage.h"

#include <QWidget>
#include <QVector>

class QTableWidget;

class DiagnosticPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit DiagnosticPanel(QWidget *parent = nullptr);

    void setDiagnostics(const QVector<DiagnosticMessage> &diagnostics);
    void clear();

private:
    QTableWidget *m_table = nullptr;
};
