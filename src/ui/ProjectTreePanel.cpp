#include "ProjectTreePanel.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

namespace
{
constexpr int ItemTypeRole = Qt::UserRole + 1;
constexpr int GeometryNameRole = Qt::UserRole + 2;
constexpr int MeshNameRole = Qt::UserRole + 3;
constexpr int GeometryItemType = 1;
constexpr int MeshItemType = 2;
constexpr int CategoryItemType = 3;

enum class CategoryType
{
    Material = 1,
    BoundaryCondition = 2,
    Load = 3,
    Solver = 4
};
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

void ProjectTreePanel::setMeshItems(const QStringList &meshNames)
{
    if (!m_meshRoot) {
        return;
    }

    while (m_meshRoot->childCount() > 0) {
        delete m_meshRoot->takeChild(0);
    }
    for (const QString &meshName : meshNames) {
        auto *item = new QTreeWidgetItem(QStringList{meshName});
        item->setData(0, ItemTypeRole, MeshItemType);
        item->setData(0, MeshNameRole, meshName);
        m_meshRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::buildInitialTree()
{
    buildProjectTree("Unnamed Project", "");
}

void ProjectTreePanel::buildProjectTree(const QString &projectName, const QString &projectPath)
{
    auto *projectRoot = new QTreeWidgetItem(QStringList{"Unnamed Project"});
    projectRoot->setText(0, projectName);
    if (!projectPath.isEmpty()) {
        projectRoot->setToolTip(0, projectPath);
    }

    m_tree->clear();
    m_geometryRoot = new QTreeWidgetItem(QStringList{"Geometry"});
    projectRoot->addChild(m_geometryRoot);

    m_materialRoot = new QTreeWidgetItem(QStringList{"Materials"});
    m_materialRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_materialRoot->setData(0, Qt::UserRole + 10, static_cast<int>(CategoryType::Material));
    projectRoot->addChild(m_materialRoot);

    m_boundaryConditionRoot = new QTreeWidgetItem(QStringList{"Boundary Conditions"});
    m_boundaryConditionRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_boundaryConditionRoot->setData(0, Qt::UserRole + 10, static_cast<int>(CategoryType::BoundaryCondition));
    projectRoot->addChild(m_boundaryConditionRoot);

    m_loadRoot = new QTreeWidgetItem(QStringList{"Loads"});
    m_loadRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_loadRoot->setData(0, Qt::UserRole + 10, static_cast<int>(CategoryType::Load));
    projectRoot->addChild(m_loadRoot);

    m_meshRoot = new QTreeWidgetItem(QStringList{"Mesh"});
    projectRoot->addChild(m_meshRoot);

    m_solverRoot = new QTreeWidgetItem(QStringList{"Solver"});
    m_solverRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_solverRoot->setData(0, Qt::UserRole + 10, static_cast<int>(CategoryType::Solver));
    projectRoot->addChild(m_solverRoot);

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
    } else if (current->data(0, ItemTypeRole).toInt() == MeshItemType) {
        emit meshSelected(current->data(0, MeshNameRole).toString());
    } else if (current->data(0, ItemTypeRole).toInt() == CategoryItemType) {
        const int categoryType = current->data(0, Qt::UserRole + 10).toInt();
        switch (static_cast<CategoryType>(categoryType)) {
        case CategoryType::Material:
            emit materialCategorySelected();
            break;
        case CategoryType::BoundaryCondition:
            emit boundaryConditionCategorySelected();
            break;
        case CategoryType::Load:
            emit loadCategorySelected();
            break;
        case CategoryType::Solver:
            emit solverCategorySelected();
            break;
        }
    }
}
