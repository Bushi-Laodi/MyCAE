#pragma once

#include "project/ProjectResourceManager.h"

#include <QDialog>
#include <QStringList>

class QLabel;
class QPushButton;
class QTableWidget;
class ProjectModel;

class ProjectResourceDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectResourceDialog(ProjectModel &projectModel, QWidget *parent = nullptr);

signals:
    void logMessagesReady(const QStringList &messages);
    void resultsChanged();

private:
    void refresh();
    void openProjectDirectory();
    void removeInvalidResultRecords();
    void deleteSelectedResultFiles();
    QString selectedResultId() const;

    ProjectModel &m_projectModel;
    ProjectResourceManager m_manager;
    ProjectResourceReport m_report;
    QLabel *m_summaryLabel = nullptr;
    QTableWidget *m_table = nullptr;
    QPushButton *m_removeInvalidResultsButton = nullptr;
    QPushButton *m_deleteSelectedFilesButton = nullptr;
};
