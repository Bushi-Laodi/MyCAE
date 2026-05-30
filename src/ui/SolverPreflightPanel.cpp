#include "ui/SolverPreflightPanel.h"

#include <QHeaderView>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString statusStyle(const SolverPreflightReport &report)
{
    if (report.hasFailed()) {
        return "QLabel { color: #991b1b; background: #fee2e2; border: 1px solid #fecaca; border-radius: 4px; padding: 6px; }";
    }
    if (report.hasWarning()) {
        return "QLabel { color: #92400e; background: #fef3c7; border: 1px solid #fde68a; border-radius: 4px; padding: 6px; }";
    }
    return "QLabel { color: #166534; background: #dcfce7; border: 1px solid #bbf7d0; border-radius: 4px; padding: 6px; }";
}

QString statusPrefix(PreflightCheckStatus status)
{
    switch (status) {
    case PreflightCheckStatus::Passed:
        return zh(u8"通过");
    case PreflightCheckStatus::Warning:
        return zh(u8"警告");
    case PreflightCheckStatus::Failed:
        return zh(u8"失败");
    }
    return zh(u8"通过");
}
}

SolverPreflightPanel::SolverPreflightPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    m_summaryLabel = new QLabel(zh(u8"尚未运行求解前检查。"), this);
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setStyleSheet("QLabel { color: #374151; background: #f3f4f6; border: 1px solid #d1d5db; border-radius: 4px; padding: 6px; }");
    layout->addWidget(m_summaryLabel);

    m_tree = new QTreeWidget(this);
    m_tree->setObjectName("solverPreflight.tree");
    m_tree->setColumnCount(3);
    m_tree->setHeaderLabels({zh(u8"检查项"), zh(u8"状态"), zh(u8"建议")});
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tree->setRootIsDecorated(false);
    m_tree->setAlternatingRowColors(true);
    layout->addWidget(m_tree);
}

void SolverPreflightPanel::setReport(const SolverPreflightReport &report)
{
    m_summaryLabel->setText(report.summary());
    m_summaryLabel->setStyleSheet(statusStyle(report));
    m_tree->clear();

    for (const PreflightCheckItem &item : report.items) {
        auto *row = new QTreeWidgetItem(m_tree);
        row->setText(0, item.category + zh(u8"：") + item.message);
        row->setText(1, statusPrefix(item.status));
        row->setText(2, item.suggestion.trimmed().isEmpty() ? QStringLiteral("-") : item.suggestion);
    }
    m_tree->resizeColumnToContents(1);
}

void SolverPreflightPanel::clear()
{
    m_summaryLabel->setText(zh(u8"尚未运行求解前检查。"));
    m_summaryLabel->setStyleSheet("QLabel { color: #374151; background: #f3f4f6; border: 1px solid #d1d5db; border-radius: 4px; padding: 6px; }");
    m_tree->clear();
}
