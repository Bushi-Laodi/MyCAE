#include "ui/ProjectResourceDialog.h"

#include "project/ProjectModel.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
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

ProjectResourceDialog::ProjectResourceDialog(ProjectModel &projectModel, QWidget *parent)
    : QDialog(parent)
    , m_projectModel(projectModel)
{
    setWindowTitle("Project Resources");
    resize(980, 520);

    auto *layout = new QVBoxLayout(this);
    m_summaryLabel = new QLabel(this);
    layout->addWidget(m_summaryLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Category", "Name", "Status", "Size", "Path", "Detail"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_table);

    auto *buttons = new QDialogButtonBox(this);
    auto *refreshButton = buttons->addButton("Refresh", QDialogButtonBox::ActionRole);
    auto *openProjectButton = buttons->addButton("Open Project Directory", QDialogButtonBox::ActionRole);
    m_removeInvalidResultsButton = buttons->addButton("Remove Invalid Result Records", QDialogButtonBox::ActionRole);
    m_deleteSelectedFilesButton = buttons->addButton("Delete Selected Result Files", QDialogButtonBox::ActionRole);
    auto *closeButton = buttons->addButton(QDialogButtonBox::Close);
    connect(refreshButton, &QPushButton::clicked, this, &ProjectResourceDialog::refresh);
    connect(openProjectButton, &QPushButton::clicked, this, &ProjectResourceDialog::openProjectDirectory);
    connect(m_removeInvalidResultsButton, &QPushButton::clicked, this, &ProjectResourceDialog::removeInvalidResultRecords);
    connect(m_deleteSelectedFilesButton, &QPushButton::clicked, this, &ProjectResourceDialog::deleteSelectedResultFiles);
    connect(closeButton, &QPushButton::clicked, this, &ProjectResourceDialog::accept);
    layout->addWidget(buttons);

    refresh();
}

void ProjectResourceDialog::refresh()
{
    m_report = m_manager.inspect(m_projectModel);
    m_table->setRowCount(m_report.items.size());
    for (int row = 0; row < m_report.items.size(); ++row) {
        const ProjectResourceItem &resource = m_report.items.at(row);
        m_table->setItem(row, 0, item(resource.category));
        m_table->setItem(row, 1, item(resource.name));
        m_table->setItem(row, 2, item(projectResourceStatusName(resource.status)));
        m_table->setItem(row, 3, item(projectResourceSizeText(resource.sizeBytes)));
        m_table->setItem(row, 4, item(QDir::toNativeSeparators(resource.path)));
        m_table->setItem(row, 5, item(resource.detail));
        m_table->item(row, 0)->setData(Qt::UserRole, resource.resultId);
    }

    m_summaryLabel->setText(QString("Resources: %1, invalid results: %2, disk usage: %3")
        .arg(m_report.items.size())
        .arg(m_report.invalidResultIds.size())
        .arg(projectResourceSizeText(m_report.totalBytes)));
    m_removeInvalidResultsButton->setEnabled(!m_report.invalidResultIds.isEmpty());
    m_deleteSelectedFilesButton->setEnabled(m_projectModel.hasProject());
}

void ProjectResourceDialog::openProjectDirectory()
{
    if (m_projectModel.hasProject()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_projectModel.project().rootPath));
    }
}

void ProjectResourceDialog::removeInvalidResultRecords()
{
    if (m_report.invalidResultIds.isEmpty()) {
        return;
    }
    if (QMessageBox::question(this, "Remove Invalid Result Records", "Remove invalid result records from history?")
        != QMessageBox::Yes) {
        emit logMessagesReady({"Remove invalid result records canceled."});
        return;
    }

    QStringList removedIds;
    QString errorMessage;
    if (!m_manager.removeInvalidResultRecords(m_projectModel, &removedIds, &errorMessage)) {
        emit logMessagesReady({"Remove invalid result records failed: " + errorMessage});
        return;
    }
    emit logMessagesReady({"Invalid result records removed: " + removedIds.join(", ")});
    emit resultsChanged();
    refresh();
}

void ProjectResourceDialog::deleteSelectedResultFiles()
{
    const QString resultId = selectedResultId();
    if (resultId.isEmpty()) {
        emit logMessagesReady({"Delete result files skipped: select a result row first."});
        return;
    }
    if (QMessageBox::question(this, "Delete Result Files", "Delete files on disk for result \"" + resultId + "\"?")
        != QMessageBox::Yes) {
        emit logMessagesReady({"Delete result files canceled."});
        return;
    }

    QString errorMessage;
    if (!m_manager.deleteResultFiles(m_projectModel, resultId, &errorMessage)) {
        emit logMessagesReady({"Delete result files failed: " + errorMessage});
        return;
    }
    emit logMessagesReady({"Result files deleted: " + resultId});
    refresh();
}

QString ProjectResourceDialog::selectedResultId() const
{
    const QList<QTableWidgetItem *> selected = m_table->selectedItems();
    if (selected.isEmpty()) {
        return {};
    }
    const int row = selected.first()->row();
    QTableWidgetItem *categoryItem = m_table->item(row, 0);
    if (!categoryItem || categoryItem->text() != "Result") {
        return {};
    }
    return categoryItem->data(Qt::UserRole).toString();
}
