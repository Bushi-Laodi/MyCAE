#include "ui/DiagnosticPanel.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace
{
QTableWidgetItem *item(const QString &text)
{
    auto *tableItem = new QTableWidgetItem(text);
    tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
    return tableItem;
}
}

DiagnosticPanel::DiagnosticPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Severity", "Category", "Message", "Suggested Fix"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setWordWrap(true);
    layout->addWidget(m_table);
}

void DiagnosticPanel::setDiagnostics(const QVector<DiagnosticMessage> &diagnostics)
{
    m_table->setRowCount(diagnostics.size());
    for (int row = 0; row < diagnostics.size(); ++row) {
        const DiagnosticMessage &diagnostic = diagnostics.at(row);
        m_table->setItem(row, 0, item(diagnosticSeverityName(diagnostic.severity)));
        m_table->setItem(row, 1, item(diagnosticCategoryName(diagnostic.category)));
        m_table->setItem(row, 2, item(diagnostic.message));
        m_table->setItem(row, 3, item(diagnostic.suggestedFix));
    }
    m_table->resizeRowsToContents();
}

void DiagnosticPanel::clear()
{
    m_table->setRowCount(0);
}

