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
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString categoryDisplayName(const QString &category)
{
    if (category == "Project") {
        return zh(u8"工程");
    }
    if (category == "Geometry") {
        return zh(u8"几何");
    }
    if (category == "Mesh") {
        return zh(u8"网格");
    }
    if (category == "Result") {
        return zh(u8"结果");
    }
    return category;
}

QString statusDisplayName(ProjectResourceStatus status)
{
    switch (status) {
    case ProjectResourceStatus::Valid:
        return zh(u8"正常");
    case ProjectResourceStatus::Missing:
        return zh(u8"缺失");
    case ProjectResourceStatus::Incomplete:
        return zh(u8"不完整");
    }
    return zh(u8"未知");
}

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
    setWindowTitle(zh(u8"工程资源"));
    resize(980, 520);

    auto *layout = new QVBoxLayout(this);
    m_summaryLabel = new QLabel(this);
    layout->addWidget(m_summaryLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels(
        {zh(u8"类别"), zh(u8"名称"), zh(u8"状态"), zh(u8"大小"), zh(u8"路径"), zh(u8"详情")}
    );
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
    auto *refreshButton = buttons->addButton(zh(u8"刷新"), QDialogButtonBox::ActionRole);
    auto *openProjectButton = buttons->addButton(zh(u8"打开工程目录"), QDialogButtonBox::ActionRole);
    m_removeInvalidResultsButton = buttons->addButton(zh(u8"移除无效结果记录"), QDialogButtonBox::ActionRole);
    m_deleteSelectedFilesButton = buttons->addButton(zh(u8"删除所选结果文件"), QDialogButtonBox::ActionRole);
    auto *closeButton = buttons->addButton(QDialogButtonBox::Close);
    closeButton->setText(zh(u8"关闭"));
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
        m_table->setItem(row, 0, item(categoryDisplayName(resource.category)));
        m_table->setItem(row, 1, item(resource.name));
        m_table->setItem(row, 2, item(statusDisplayName(resource.status)));
        m_table->setItem(row, 3, item(projectResourceSizeText(resource.sizeBytes)));
        m_table->setItem(row, 4, item(QDir::toNativeSeparators(resource.path)));
        m_table->setItem(row, 5, item(resource.detail));
        m_table->item(row, 0)->setData(Qt::UserRole, resource.resultId);
    }

    m_summaryLabel->setText(zh(u8"资源：%1，无效结果：%2，磁盘占用：%3")
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
    if (QMessageBox::question(this, zh(u8"移除无效结果记录"), zh(u8"从历史记录中移除无效结果记录？"))
        != QMessageBox::Yes) {
        emit logMessagesReady({zh(u8"已取消移除无效结果记录。")});
        return;
    }

    QStringList removedIds;
    QString errorMessage;
    if (!m_manager.removeInvalidResultRecords(m_projectModel, &removedIds, &errorMessage)) {
        emit logMessagesReady({zh(u8"移除无效结果记录失败：") + errorMessage});
        return;
    }
    emit logMessagesReady({zh(u8"已移除无效结果记录：") + removedIds.join(", ")});
    emit resultsChanged();
    refresh();
}

void ProjectResourceDialog::deleteSelectedResultFiles()
{
    const QString resultId = selectedResultId();
    if (resultId.isEmpty()) {
        emit logMessagesReady({zh(u8"未删除结果文件：请先选择一条结果记录。")});
        return;
    }
    if (QMessageBox::question(this, zh(u8"删除结果文件"), zh(u8"删除结果“%1”在磁盘上的文件？").arg(resultId))
        != QMessageBox::Yes) {
        emit logMessagesReady({zh(u8"已取消删除结果文件。")});
        return;
    }

    QString errorMessage;
    if (!m_manager.deleteResultFiles(m_projectModel, resultId, &errorMessage)) {
        emit logMessagesReady({zh(u8"删除结果文件失败：") + errorMessage});
        return;
    }
    emit logMessagesReady({zh(u8"已删除结果文件：") + resultId});
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
    if (!categoryItem) {
        return {};
    }
    return categoryItem->data(Qt::UserRole).toString();
}
