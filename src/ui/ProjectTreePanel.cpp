#include "ProjectTreePanel.h"

#include "geometry/FaceGroup.h"
#include "result/ResultObject.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QSize>

namespace
{
constexpr int SelectionKindRole = Qt::UserRole + 1;
constexpr int SelectionIdRole = Qt::UserRole + 2;
constexpr int SelectionNameRole = Qt::UserRole + 3;

QIcon badgeIcon(const QString &text, const QColor &background, const QColor &foreground = Qt::white)
{
    constexpr int size = 18;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(background);
    painter.drawRoundedRect(1, 1, size - 2, size - 2, 4, 4);

    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(text.size() > 1 ? 7 : 9);
    painter.setFont(font);
    painter.setPen(foreground);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    return QIcon(pixmap);
}

void applyRootStyle(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    item->setIcon(0, badgeIcon("P", QColor("#2563eb")));
    item->setBackground(0, QBrush(QColor("#eef4ff")));
    item->setForeground(0, QBrush(QColor("#111827")));
}

void applyCategoryStyle(QTreeWidgetItem *item, const QIcon &icon)
{
    if (!item) {
        return;
    }
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    item->setIcon(0, icon);
    item->setBackground(0, QBrush(QColor("#f3f4f6")));
    item->setForeground(0, QBrush(QColor("#374151")));
}

void applyLeafStyle(QTreeWidgetItem *item, const QIcon &icon)
{
    if (!item) {
        return;
    }
    item->setIcon(0, icon);
    item->setForeground(0, QBrush(QColor("#111827")));
}

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
    m_tree->setIndentation(22);
    m_tree->setIconSize(QSize(16, 16));
    m_tree->setUniformRowHeights(true);
    m_tree->setAnimated(true);
    m_tree->setStyleSheet(
        "QTreeWidget {"
        "  background: #ffffff;"
        "  border: 0;"
        "  outline: 0;"
        "}"
        "QTreeWidget::item {"
        "  min-height: 22px;"
        "  padding: 2px 4px;"
        "}"
        "QTreeWidget::item:selected {"
        "  background: #dbeafe;"
        "  color: #111827;"
        "}"
        "QTreeWidget::branch {"
        "  background: transparent;"
        "}"
    );
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
        applyLeafStyle(item, badgeIcon("G", QColor("#2563eb")));
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
        applyLeafStyle(item, badgeIcon("M", QColor("#0891b2")));
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
        applyLeafStyle(item, badgeIcon("F", QColor("#7c3aed")));
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
        applyLeafStyle(item, badgeIcon("Mt", QColor("#16a34a")));
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
        applyLeafStyle(item, badgeIcon("BC", QColor("#ca8a04")));
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
        applyLeafStyle(item, badgeIcon("L", QColor("#dc2626")));
        m_loadRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::setResultItems(const std::vector<ResultObject> &results)
{
    if (!m_resultRoot) {
        return;
    }

    clearChildren(m_resultRoot);
    for (const ResultObject &resultObject : results) {
        auto *item = new QTreeWidgetItem(QStringList{resultObject.name});
        setItemSelection(item, Selection::item(SelectionKind::Result, resultObject.id, resultObject.name));
        item->setToolTip(0, resultObject.summary);
        applyLeafStyle(item, badgeIcon("R", QColor("#4f46e5")));
        m_resultRoot->addChild(item);
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
    applyRootStyle(projectRoot);

    m_tree->clear();
    m_geometryRoot = new QTreeWidgetItem(QStringList{"Geometry"});
    applyCategoryStyle(m_geometryRoot, badgeIcon("G", QColor("#2563eb")));
    projectRoot->addChild(m_geometryRoot);

    m_faceGroupRoot = new QTreeWidgetItem(QStringList{"Face Groups"});
    applyCategoryStyle(m_faceGroupRoot, badgeIcon("F", QColor("#7c3aed")));
    projectRoot->addChild(m_faceGroupRoot);

    m_materialRoot = new QTreeWidgetItem(QStringList{"Materials"});
    setItemSelection(m_materialRoot, Selection::category(SelectionKind::MaterialCategory));
    applyCategoryStyle(m_materialRoot, badgeIcon("Mt", QColor("#16a34a")));
    projectRoot->addChild(m_materialRoot);

    m_boundaryConditionRoot = new QTreeWidgetItem(QStringList{"Boundary Conditions"});
    setItemSelection(m_boundaryConditionRoot, Selection::category(SelectionKind::BoundaryConditionCategory));
    applyCategoryStyle(m_boundaryConditionRoot, badgeIcon("BC", QColor("#ca8a04")));
    projectRoot->addChild(m_boundaryConditionRoot);

    m_loadRoot = new QTreeWidgetItem(QStringList{"Loads"});
    setItemSelection(m_loadRoot, Selection::category(SelectionKind::LoadCategory));
    applyCategoryStyle(m_loadRoot, badgeIcon("L", QColor("#dc2626")));
    projectRoot->addChild(m_loadRoot);

    m_meshRoot = new QTreeWidgetItem(QStringList{"Mesh"});
    applyCategoryStyle(m_meshRoot, badgeIcon("M", QColor("#0891b2")));
    projectRoot->addChild(m_meshRoot);

    m_solverRoot = new QTreeWidgetItem(QStringList{"Solver"});
    setItemSelection(m_solverRoot, Selection::category(SelectionKind::SolverCategory));
    applyCategoryStyle(m_solverRoot, badgeIcon("S", QColor("#475569")));
    projectRoot->addChild(m_solverRoot);

    m_resultRoot = new QTreeWidgetItem(QStringList{"Results"});
    setItemSelection(m_resultRoot, Selection::category(SelectionKind::ResultCategory));
    applyCategoryStyle(m_resultRoot, badgeIcon("R", QColor("#4f46e5")));
    projectRoot->addChild(m_resultRoot);

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
