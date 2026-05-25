#pragma once

#include "project/SelectionState.h"

#include <QString>
#include <QStringList>
#include <QWidget>

#include <vector>

class QTreeWidget;
class QTreeWidgetItem;
struct BoundaryCondition;
struct FaceGroup;
struct Load;
struct Material;

class ProjectTreePanel final : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectTreePanel(QWidget *parent = nullptr);

    void showProject(const QString &projectName, const QString &projectPath);
    void setGeometryItems(const QStringList &geometryNames);
    void setMeshItems(const QStringList &meshNames);
    void setFaceGroupItems(const std::vector<FaceGroup> &faceGroups);
    void setMaterialItems(const std::vector<Material> &materials);
    void setBoundaryConditionItems(const std::vector<BoundaryCondition> &boundaryConditions);
    void setLoadItems(const std::vector<Load> &loads);

signals:
    void selectionChanged(const Selection &selection);

private:
    void buildInitialTree();
    void buildProjectTree(const QString &projectName, const QString &projectPath);
    void handleCurrentItemChanged(QTreeWidgetItem *current);
    void setItemSelection(QTreeWidgetItem *item, const Selection &selection);

    QTreeWidget *m_tree = nullptr;
    QTreeWidgetItem *m_geometryRoot = nullptr;
    QTreeWidgetItem *m_faceGroupRoot = nullptr;
    QTreeWidgetItem *m_meshRoot = nullptr;
    QTreeWidgetItem *m_materialRoot = nullptr;
    QTreeWidgetItem *m_boundaryConditionRoot = nullptr;
    QTreeWidgetItem *m_loadRoot = nullptr;
    QTreeWidgetItem *m_solverRoot = nullptr;
};
