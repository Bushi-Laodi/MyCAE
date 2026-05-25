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
constexpr int SelectionKindRole = Qt::UserRole + 1;
constexpr int SelectionIdRole = Qt::UserRole + 2;
constexpr int SelectionNameRole = Qt::UserRole + 3;

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
        setItemSelection(item, Selection::item(SelectionKind::Geometry, geometryName, geometryName));
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
        setItemSelection(item, Selection::item(SelectionKind::Mesh, meshName, meshName));
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
        auto *item = new QTreeWidgetItem(QStringList{FaceGroups::displayName(faceGroup)});
        setItemSelection(item, Selection::item(SelectionKind::FaceGroup, faceGroup.id, FaceGroups::displayName(faceGroup)));
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
        setItemSelection(item, Selection::item(SelectionKind::Material, material.id, material.name));
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
        setItemSelection(
            item,
            Selection::item(SelectionKind::BoundaryCondition, boundaryCondition.id, boundaryCondition.name)
        );
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
        setItemSelection(item, Selection::item(SelectionKind::Load, load.id, load.name));
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
    setItemSelection(m_materialRoot, Selection::category(SelectionKind::MaterialCategory));
    projectRoot->addChild(m_materialRoot);

    m_boundaryConditionRoot = new QTreeWidgetItem(QStringList{"Boundary Conditions"});
    setItemSelection(m_boundaryConditionRoot, Selection::category(SelectionKind::BoundaryConditionCategory));
    projectRoot->addChild(m_boundaryConditionRoot);

    m_loadRoot = new QTreeWidgetItem(QStringList{"Loads"});
    setItemSelection(m_loadRoot, Selection::category(SelectionKind::LoadCategory));
    projectRoot->addChild(m_loadRoot);

    m_meshRoot = new QTreeWidgetItem(QStringList{"Mesh"});
    projectRoot->addChild(m_meshRoot);

    m_solverRoot = new QTreeWidgetItem(QStringList{"Solver"});
    setItemSelection(m_solverRoot, Selection::category(SelectionKind::SolverCategory));
    projectRoot->addChild(m_solverRoot);

    m_tree->addTopLevelItem(projectRoot);
    m_tree->expandAll();
}

void ProjectTreePanel::setItemSelection(QTreeWidgetItem *item, const Selection &selection)
{
    if (!item) {
        return;
    }

    item->setData(0, SelectionKindRole, static_cast<int>(selection.kind));
    item->setData(0, SelectionIdRole, selection.id);
    item->setData(0, SelectionNameRole, selection.displayName);
}

void ProjectTreePanel::handleCurrentItemChanged(QTreeWidgetItem *current)
{
    if (!current) {
        return;
    }

    const SelectionKind kind = static_cast<SelectionKind>(current->data(0, SelectionKindRole).toInt());
    if (kind == SelectionKind::None) {
        return;
    }

    Selection selection;
    selection.kind = kind;
    selection.id = current->data(0, SelectionIdRole).toString();
    selection.displayName = current->data(0, SelectionNameRole).toString();
    emit selectionChanged(selection);
}
