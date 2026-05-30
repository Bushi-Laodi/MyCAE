#pragma once

#include "solver/SolverPreflightReport.h"

#include <QWidget>

class QLabel;
class QTreeWidget;

class SolverPreflightPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit SolverPreflightPanel(QWidget *parent = nullptr);

    void setReport(const SolverPreflightReport &report);
    void clear();

private:
    QLabel *m_summaryLabel = nullptr;
    QTreeWidget *m_tree = nullptr;
};
