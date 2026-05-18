#include "ProjectTreePanel.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

namespace
{
constexpr int ItemTypeRole = Qt::UserRole + 1;
constexpr int GeometryNameRole = Qt::UserRole + 2;
constexpr int GeometryItemType = 1;
}

ProjectTreePanel::ProjectTreePanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    layout->addWidget(m_tree);
    connect(m_tree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *current) {
        handleCurrentItemChanged(current);
    });

    buildInitialTree();
}

void ProjectTreePanel::showProject(const QString &projectName, const QString &projectPath)
{
    buildProjectTree(projectName, projectPath);
}

void ProjectTreePanel::setGeometryItems(const QStringList &geometryNames)
{
    if (!m_geometryRoot) {
        return;
    }

    while (m_geometryRoot->childCount() > 0) {
        delete m_geometryRoot->takeChild(0);
    }
    for (const QString &geometryName : geometryNames) {
        auto *item = new QTreeWidgetItem(QStringList{geometryName});
        item->setData(0, ItemTypeRole, GeometryItemType);
        item->setData(0, GeometryNameRole, geometryName);
        m_geometryRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::buildInitialTree()
{
    buildProjectTree("未命名工程", "");
}

void ProjectTreePanel::buildProjectTree(const QString &projectName, const QString &projectPath)
{
    auto *projectRoot = new QTreeWidgetItem(QStringList{"未命名工程"});
    projectRoot->setText(0, projectName);
    if (!projectPath.isEmpty()) {
        projectRoot->setToolTip(0, projectPath);
    }

    m_tree->clear();
    m_geometryRoot = new QTreeWidgetItem(QStringList{"几何"});
    projectRoot->addChild(m_geometryRoot);
    projectRoot->addChild(new QTreeWidgetItem(QStringList{"材料"}));
    projectRoot->addChild(new QTreeWidgetItem(QStringList{"边界条件"}));
    projectRoot->addChild(new QTreeWidgetItem(QStringList{"载荷"}));
    projectRoot->addChild(new QTreeWidgetItem(QStringList{"网格"}));
    projectRoot->addChild(new QTreeWidgetItem(QStringList{"求解器"}));

    m_tree->addTopLevelItem(projectRoot);
    m_tree->expandAll();
}

void ProjectTreePanel::handleCurrentItemChanged(QTreeWidgetItem *current)
{
    if (!current) {
        return;
    }

    if (current->data(0, ItemTypeRole).toInt() == GeometryItemType) {
        emit geometrySelected(current->data(0, GeometryNameRole).toString());
    }
}
