#include "ui/DiagnosticPanel.h"

#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QColor severityForeground(DiagnosticSeverity severity)
{
    switch (severity) {
    case DiagnosticSeverity::Error:
        return QColor("#991b1b");
    case DiagnosticSeverity::Warning:
        return QColor("#92400e");
    case DiagnosticSeverity::Info:
        return QColor("#1e3a8a");
    }
    return QColor("#374151");
}

QColor severityBackground(DiagnosticSeverity severity)
{
    switch (severity) {
    case DiagnosticSeverity::Error:
        return QColor("#fee2e2");
    case DiagnosticSeverity::Warning:
        return QColor("#fef3c7");
    case DiagnosticSeverity::Info:
        return QColor("#dbeafe");
    }
    return QColor("#f3f4f6");
}

QTableWidgetItem *item(const QString &text, DiagnosticSeverity severity = DiagnosticSeverity::Info, bool colorize = false)
{
    auto *tableItem = new QTableWidgetItem(text);
    tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
    if (colorize) {
        tableItem->setForeground(QBrush(severityForeground(severity)));
        tableItem->setBackground(QBrush(severityBackground(severity)));
    }
    return tableItem;
}

void setEmptyDiagnostics(QTableWidget *table)
{
    if (!table) {
        return;
    }
    table->setRowCount(1);
    table->setItem(0, 0, item(zh(u8"信息"), DiagnosticSeverity::Info, true));
    table->setItem(0, 1, item("UI"));
    table->setItem(0, 2, item(zh(u8"暂无诊断信息。")));
    table->setItem(0, 3, item("-"));
    table->resizeRowsToContents();
}
}

DiagnosticPanel::DiagnosticPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    m_table = new QTableWidget(this);
    m_table->setObjectName("diagnostic.table");
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({zh(u8"级别"), zh(u8"类别"), zh(u8"消息"), zh(u8"建议修复")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setWordWrap(true);
    setEmptyDiagnostics(m_table);

    layout->addWidget(m_table);
}

void DiagnosticPanel::setDiagnostics(const QVector<DiagnosticMessage> &diagnostics)
{
    if (diagnostics.isEmpty()) {
        setEmptyDiagnostics(m_table);
        return;
    }

    m_table->setRowCount(diagnostics.size());
    for (int row = 0; row < diagnostics.size(); ++row) {
        const DiagnosticMessage &diagnostic = diagnostics.at(row);
        m_table->setItem(row, 0, item(diagnosticSeverityName(diagnostic.severity), diagnostic.severity, true));
        m_table->setItem(row, 1, item(diagnosticCategoryName(diagnostic.category)));
        m_table->setItem(row, 2, item(diagnostic.message));
        m_table->setItem(row, 3, item(diagnostic.suggestedFix));
    }
    m_table->resizeRowsToContents();
}

void DiagnosticPanel::clear()
{
    setEmptyDiagnostics(m_table);
}
