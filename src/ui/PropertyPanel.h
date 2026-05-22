#pragma once

#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"

#include <QWidget>

#include <vector>

class QLabel;
struct BoxGeometry;
struct CylinderGeometry;
struct MeshObject;

class PropertyPanel final : public QWidget
{
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void showEmptySelection();
    void showBoxGeometry(const BoxGeometry &box);
    void showCylinderGeometry(const CylinderGeometry &cylinder);
    void showMeshObject(const MeshObject &meshObject);

    void showMaterialCategory(const std::vector<Material> &materials);
    void showBoundaryConditionCategory(const std::vector<BoundaryCondition> &boundaryConditions);
    void showLoadCategory(const std::vector<Load> &loads);
    void showSolverCategory();

private:
    void clearAll();
    void addLabelRow(const QString &label, const QString &value);

    QLabel *m_selectionValue = nullptr;
    QLabel *m_typeValue = nullptr;
    QLabel *m_nameValue = nullptr;
    QLabel *m_radiusValue = nullptr;
    QLabel *m_lengthValue = nullptr;
    QLabel *m_widthValue = nullptr;
    QLabel *m_heightValue = nullptr;
    QLabel *m_sourceGeometryValue = nullptr;
    QLabel *m_sourceGeometryTypeValue = nullptr;
    QLabel *m_sourceStepFileValue = nullptr;
    QLabel *m_meshFileValue = nullptr;
    QLabel *m_nodeCountValue = nullptr;
    QLabel *m_tetraCountValue = nullptr;
    QLabel *m_createdAtValue = nullptr;

    // Dynamic content area for solver data
    QWidget *m_dynamicArea = nullptr;
};
