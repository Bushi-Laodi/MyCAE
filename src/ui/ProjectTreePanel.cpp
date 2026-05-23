#include "ProjectTreePanel.h"

#include "geometry/FaceGroup.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

namespace
{
constexpr int ItemTypeRole = Qt::UserRole + 1;
constexpr int GeometryNameRole = Qt::UserRole + 2;
constexpr int MeshNameRole = Qt::UserRole + 3;
constexpr int SolverDataIdRole = Qt::UserRole + 4;
constexpr int FaceGroupIdRole = Qt::UserRole + 5;
constexpr int CategoryRole = Qt::UserRole + 10;
constexpr int GeometryItemType = 1;
constexpr int MeshItemType = 2;
constexpr int CategoryItemType = 3;
constexpr int MaterialItemType = 4;
constexpr int BoundaryConditionItemType = 5;
constexpr int LoadItemType = 6;
constexpr int FaceGroupItemType = 7;

enum class CategoryType
{
    Material = 1,
    BoundaryCondition = 2,
    Load = 3,
    Solver = 4
};

void clearChildren(QTreeWidgetItem *root)
{
    if (!root) {
        return;
    }

    while (root->childCount() > 0) {
        delete root->takeChild(0);
    }
}
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

    clearChildren(m_geometryRoot);
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

    clearChildren(m_meshRoot);
    for (const QString &meshName : meshNames) {
        auto *item = new QTreeWidgetItem(QStringList{meshName});
        item->setData(0, ItemTypeRole, MeshItemType);
        item->setData(0, MeshNameRole, meshName);
        m_meshRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::setFaceGroupItems(const std::vector<FaceGroup> &faceGroups)
{
    if (!m_faceGroupRoot) {
        return;
    }

    clearChildren(m_faceGroupRoot);
    for (const FaceGroup &faceGroup : faceGroups) {
        const QString label = faceGroup.geometryName + "." + faceGroup.name;
        auto *item = new QTreeWidgetItem(QStringList{label});
        item->setData(0, ItemTypeRole, FaceGroupItemType);
        item->setData(0, FaceGroupIdRole, faceGroup.id);
        item->setToolTip(0, faceGroup.id);
        m_faceGroupRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::setMaterialItems(const std::vector<Material> &materials)
{
    if (!m_materialRoot) {
        return;
    }

    clearChildren(m_materialRoot);
    for (const Material &material : materials) {
        auto *item = new QTreeWidgetItem(QStringList{material.name});
        item->setData(0, ItemTypeRole, MaterialItemType);
        item->setData(0, SolverDataIdRole, material.id);
        item->setToolTip(0, material.id);
        m_materialRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::setBoundaryConditionItems(const std::vector<BoundaryCondition> &boundaryConditions)
{
    if (!m_boundaryConditionRoot) {
        return;
    }

    clearChildren(m_boundaryConditionRoot);
    for (const BoundaryCondition &boundaryCondition : boundaryConditions) {
        auto *item = new QTreeWidgetItem(QStringList{boundaryCondition.name});
        item->setData(0, ItemTypeRole, BoundaryConditionItemType);
        item->setData(0, SolverDataIdRole, boundaryCondition.id);
        item->setToolTip(0, boundaryCondition.id);
        m_boundaryConditionRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::setLoadItems(const std::vector<Load> &loads)
{
    if (!m_loadRoot) {
        return;
    }

    clearChildren(m_loadRoot);
    for (const Load &load : loads) {
        auto *item = new QTreeWidgetItem(QStringList{load.name});
        item->setData(0, ItemTypeRole, LoadItemType);
        item->setData(0, SolverDataIdRole, load.id);
        item->setToolTip(0, load.id);
        m_loadRoot->addChild(item);
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

    m_faceGroupRoot = new QTreeWidgetItem(QStringList{"Face Groups"});
    projectRoot->addChild(m_faceGroupRoot);

    m_materialRoot = new QTreeWidgetItem(QStringList{"Materials"});
    m_materialRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_materialRoot->setData(0, CategoryRole, static_cast<int>(CategoryType::Material));
    projectRoot->addChild(m_materialRoot);

    m_boundaryConditionRoot = new QTreeWidgetItem(QStringList{"Boundary Conditions"});
    m_boundaryConditionRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_boundaryConditionRoot->setData(0, CategoryRole, static_cast<int>(CategoryType::BoundaryCondition));
    projectRoot->addChild(m_boundaryConditionRoot);

    m_loadRoot = new QTreeWidgetItem(QStringList{"Loads"});
    m_loadRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_loadRoot->setData(0, CategoryRole, static_cast<int>(CategoryType::Load));
    projectRoot->addChild(m_loadRoot);

    m_meshRoot = new QTreeWidgetItem(QStringList{"Mesh"});
    projectRoot->addChild(m_meshRoot);

    m_solverRoot = new QTreeWidgetItem(QStringList{"Solver"});
    m_solverRoot->setData(0, ItemTypeRole, CategoryItemType);
    m_solverRoot->setData(0, CategoryRole, static_cast<int>(CategoryType::Solver));
    projectRoot->addChild(m_solverRoot);

    m_tree->addTopLevelItem(projectRoot);
    m_tree->expandAll();
}

void ProjectTreePanel::handleCurrentItemChanged(QTreeWidgetItem *current)
{
    if (!current) {
        return;
    }

    const int itemType = current->data(0, ItemTypeRole).toInt();
    if (itemType == GeometryItemType) {
        emit geometrySelected(current->data(0, GeometryNameRole).toString());
    } else if (itemType == MeshItemType) {
        emit meshSelected(current->data(0, MeshNameRole).toString());
    } else if (itemType == FaceGroupItemType) {
        emit faceGroupSelected(current->data(0, FaceGroupIdRole).toString());
    } else if (itemType == MaterialItemType) {
        emit materialSelected(current->data(0, SolverDataIdRole).toString());
    } else if (itemType == BoundaryConditionItemType) {
        emit boundaryConditionSelected(current->data(0, SolverDataIdRole).toString());
    } else if (itemType == LoadItemType) {
        emit loadSelected(current->data(0, SolverDataIdRole).toString());
    } else if (itemType == CategoryItemType) {
        const int categoryType = current->data(0, CategoryRole).toInt();
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
