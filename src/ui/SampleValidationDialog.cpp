#include "ui/SampleValidationDialog.h"

#include <QBrush>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace
{
QBrush statusBrush(SampleValidationStatus status)
{
    switch (status) {
    case SampleValidationStatus::Pass:
        return QBrush(QColor(33, 128, 72));
    case SampleValidationStatus::Fail:
        return QBrush(QColor(176, 42, 55));
    case SampleValidationStatus::Skip:
        return QBrush(QColor(116, 116, 116));
    }
    return QBrush(QColor(116, 116, 116));
}

QTableWidgetItem *item(const QString &text)
{
    auto *tableItem = new QTableWidgetItem(text);
    tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
    return tableItem;
}
}

SampleValidationDialog::SampleValidationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Validate Samples");
    resize(880, 420);

    auto *layout = new QVBoxLayout(this);

    m_summaryLabel = new QLabel("Ready", this);
    layout->addWidget(m_summaryLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Status", "Check", "Detail"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);

    auto *buttons = new QDialogButtonBox(this);
    m_runButton = buttons->addButton("Run", QDialogButtonBox::ActionRole);
    auto *closeButton = buttons->addButton(QDialogButtonBox::Close);
    connect(m_runButton, &QPushButton::clicked, this, &SampleValidationDialog::runValidation);
    connect(closeButton, &QPushButton::clicked, this, &SampleValidationDialog::accept);
    layout->addWidget(buttons);
}

void SampleValidationDialog::runValidation()
{
    m_runButton->setEnabled(false);
    m_summaryLabel->setText("Running...");
    m_table->setRowCount(0);

    const SampleValidationReport report = SampleProjectValidator().validate();
    setReport(report);
    emit logMessagesReady(report.logMessages);

    m_runButton->setEnabled(true);
}

void SampleValidationDialog::setReport(const SampleValidationReport &report)
{
    m_table->setRowCount(report.steps.size());
    for (int row = 0; row < report.steps.size(); ++row) {
        const SampleValidationStep &step = report.steps.at(row);
        QTableWidgetItem *statusItem = item(sampleValidationStatusName(step.status));
        statusItem->setForeground(statusBrush(step.status));
        m_table->setItem(row, 0, statusItem);
        m_table->setItem(row, 1, item(step.name));
        m_table->setItem(row, 2, item(step.detail));
    }

    m_summaryLabel->setText(report.success()
        ? QString("All sample validation checks passed: %1 pass.").arg(report.passedCount())
        : QString("Sample validation failed: %1 pass, %2 fail. Check Log and Diagnostics for details.")
            .arg(report.passedCount())
            .arg(report.failedCount()));
}
