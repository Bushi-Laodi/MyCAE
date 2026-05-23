#pragma once

#include <QString>
#include <QStringList>
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

class ProjectTreePanel final : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectTreePanel(QWidget *parent = nullptr);

    void showProject(const QString &projectName, const QString &projectPath);
    void setGeometryItems(const QStringList &geometryNames);
    void setMeshItems(const QStringList &meshNames);

signals:
    void geometrySelected(const QString &geometryName);
    void meshSelected(const QString &meshName);
    void materialCategorySelected();
    void boundaryConditionCategorySelected();
    void loadCategorySelected();
    void solverCategorySelected();

private:
    void buildInitialTree();
    void buildProjectTree(const QString &projectName, const QString &projectPath);
    void handleCurrentItemChanged(QTreeWidgetItem *current);

    QTreeWidget *m_tree = nullptr;
    QTreeWidgetItem *m_geometryRoot = nullptr;
    QTreeWidgetItem *m_meshRoot = nullptr;
    QTreeWidgetItem *m_materialRoot = nullptr;
    QTreeWidgetItem *m_boundaryConditionRoot = nullptr;
    QTreeWidgetItem *m_loadRoot = nullptr;
    QTreeWidgetItem *m_solverRoot = nullptr;
};
