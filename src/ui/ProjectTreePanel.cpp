#include "ProjectTreePanel.h"

#include "geometry/FaceGroup.h"
#include "result/ResultObject.h"
#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"
#include "ui/UiIconFactory.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QSize>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

namespace
{
constexpr int SelectionKindRole = Qt::UserRole + 1;
constexpr int SelectionIdRole = Qt::UserRole + 2;
constexpr int SelectionNameRole = Qt::UserRole + 3;

QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

int faceGroupFaceCount(const FaceGroup &faceGroup)
{
    return static_cast<int>(faceGroup.faceIndices.size());
}

QString faceGroupTreeText(const FaceGroup &faceGroup)
{
    return QString("%1 (%2 faces)")
        .arg(FaceGroups::displayName(faceGroup))
        .arg(faceGroupFaceCount(faceGroup));
}

QString projectTreeStyleSheet()
{
    return QStringLiteral(
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
}

void configureProjectTree(QTreeWidget *tree)
{
    if (!tree) {
        return;
    }
    tree->setHeaderHidden(true);
    tree->setIndentation(22);
    tree->setIconSize(QSize(16, 16));
    tree->setUniformRowHeights(true);
    tree->setAnimated(true);
    tree->setStyleSheet(projectTreeStyleSheet());
}

void applyRootStyle(QTreeWidgetItem *item)
{
    if (!item) {
        return;
    }
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    item->setIcon(0, UiIconFactory::treeBadge("P", QColor("#2563eb")));
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

QTreeWidgetItem *createStyledCategory(const QString &text, const QIcon &icon, QTreeWidgetItem *parent)
{
    auto *item = new QTreeWidgetItem(QStringList{text});
    applyCategoryStyle(item, icon);
    if (parent) {
        parent->addChild(item);
    }
    return item;
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
    configureProjectTree(m_tree);
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
        applyLeafStyle(item, UiIconFactory::treeBadge("G", QColor("#2563eb")));
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
        applyLeafStyle(item, UiIconFactory::treeBadge("M", QColor("#0891b2")));
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
        auto *item = new QTreeWidgetItem(QStringList{faceGroupTreeText(faceGroup)});
        setItemSelection(item, Selection::item(SelectionKind::FaceGroup, faceGroup.id, FaceGroups::displayName(faceGroup)));
        item->setToolTip(0, QString("%1\n%2 faces").arg(faceGroup.id).arg(faceGroupFaceCount(faceGroup)));
        applyLeafStyle(item, UiIconFactory::treeBadge("F", QColor("#7c3aed")));
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
        applyLeafStyle(item, UiIconFactory::treeBadge("Mt", QColor("#16a34a")));
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
        applyLeafStyle(item, UiIconFactory::treeBadge("BC", QColor("#ca8a04")));
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
        applyLeafStyle(item, UiIconFactory::treeBadge("L", QColor("#dc2626")));
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
        applyLeafStyle(item, UiIconFactory::treeBadge("R", QColor("#4f46e5")));
        m_resultRoot->addChild(item);
    }

    m_tree->expandAll();
}

void ProjectTreePanel::buildInitialTree()
{
    buildProjectTree(zh(u8"未命名工程"), "");
}

void ProjectTreePanel::buildProjectTree(const QString &projectName, const QString &projectPath)
{
    auto *projectRoot = new QTreeWidgetItem(QStringList{zh(u8"未命名工程")});
    projectRoot->setText(0, projectName);
    if (!projectPath.isEmpty()) {
        projectRoot->setToolTip(0, projectPath);
    }
    applyRootStyle(projectRoot);

    m_tree->clear();
    m_geometryRoot = createStyledCategory(zh(u8"几何"), UiIconFactory::treeBadge("G", QColor("#2563eb")), projectRoot);

    m_faceGroupRoot = createStyledCategory(zh(u8"面组"), UiIconFactory::treeBadge("F", QColor("#7c3aed")), projectRoot);

    m_materialRoot = createStyledCategory(zh(u8"材料"), UiIconFactory::treeBadge("Mt", QColor("#16a34a")), projectRoot);
    setItemSelection(m_materialRoot, Selection::category(SelectionKind::MaterialCategory));

    m_boundaryConditionRoot = createStyledCategory(
        zh(u8"边界条件"),
        UiIconFactory::treeBadge("BC", QColor("#ca8a04")),
        projectRoot
    );
    setItemSelection(m_boundaryConditionRoot, Selection::category(SelectionKind::BoundaryConditionCategory));

    m_loadRoot = createStyledCategory(zh(u8"载荷"), UiIconFactory::treeBadge("L", QColor("#dc2626")), projectRoot);
    setItemSelection(m_loadRoot, Selection::category(SelectionKind::LoadCategory));

    m_meshRoot = createStyledCategory(zh(u8"网格"), UiIconFactory::treeBadge("M", QColor("#0891b2")), projectRoot);

    m_solverRoot = createStyledCategory(zh(u8"求解器"), UiIconFactory::treeBadge("S", QColor("#475569")), projectRoot);
    setItemSelection(m_solverRoot, Selection::category(SelectionKind::SolverCategory));

    m_resultRoot = createStyledCategory(zh(u8"结果"), UiIconFactory::treeBadge("R", QColor("#4f46e5")), projectRoot);
    setItemSelection(m_resultRoot, Selection::category(SelectionKind::ResultCategory));

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
