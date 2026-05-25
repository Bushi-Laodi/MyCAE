#pragma once

#include "validation/SampleProjectValidator.h"

#include <QDialog>
#include <QStringList>

class QLabel;
class QPushButton;
class QTableWidget;

class SampleValidationDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SampleValidationDialog(QWidget *parent = nullptr);

public slots:
    void runValidation();

signals:
    void logMessagesReady(const QStringList &messages);

private:
    void setReport(const SampleValidationReport &report);

    QLabel *m_summaryLabel = nullptr;
    QTableWidget *m_table = nullptr;
    QPushButton *m_runButton = nullptr;
};
