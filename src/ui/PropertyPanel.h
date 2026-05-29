#pragma once

#include "solver/BoundaryCondition.h"
#include "solver/Load.h"
#include "solver/Material.h"
#include "solver/SectionAssignment.h"
#include "solver/SimulationCase.h"
#include "picking/PickMode.h"
#include "ui/property/GeometryPropertyView.h"

#include <QWidget>
#include <QString>

#include <vector>

class QLabel;
class QVBoxLayout;
struct BoxGeometry;
struct CylinderGeometry;
struct SphereGeometry;
struct FaceGroup;
struct GeometryObject;
struct MeshObject;
struct ResultObject;

class PropertyPanel final : public QWidget
{
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void showEmptySelection();
    void showBoxGeometry(const BoxGeometry &box);
    void showBoxGeometry(const BoxGeometry &box, const GeometryObject &geometry, const GeometryPropertyDetails &details);
    void showCylinderGeometry(const CylinderGeometry &cylinder);
    void showCylinderGeometry(const CylinderGeometry &cylinder, const GeometryObject &geometry, const GeometryPropertyDetails &details);
    void showSphereGeometry(const SphereGeometry &sphere);
    void showSphereGeometry(const SphereGeometry &sphere, const GeometryObject &geometry, const GeometryPropertyDetails &details);
    void showGeometryObject(const GeometryObject &geometry);
    void showGeometryObject(const GeometryObject &geometry, const GeometryPropertyDetails &details);
    void showMeshObject(const MeshObject &meshObject);
    void showFaceGroup(const FaceGroup &faceGroup);
    void showFaceGroup(
        const FaceGroup &faceGroup,
        const std::vector<BoundaryCondition> &boundaryConditions,
        const std::vector<Load> &loads
    );
    void showPickState(PickMode mode, const QString &geometryName, const std::vector<int> &faceIndices);

    void showMaterialCategory(const std::vector<Material> &materials);
    void showSectionAssignmentCategory(const std::vector<SectionAssignment> &sectionAssignments);
    void showBoundaryConditionCategory(const std::vector<BoundaryCondition> &boundaryConditions);
    void showLoadCategory(const std::vector<Load> &loads);
    void showMaterial(const Material &material);
    void showSectionAssignment(const SectionAssignment &sectionAssignment);
    void showBoundaryCondition(const BoundaryCondition &boundaryCondition);
    void showBoundaryCondition(
        const BoundaryCondition &boundaryCondition,
        const std::vector<FaceGroup> &faceGroups,
        const std::vector<Load> &loads
    );
    void showLoad(const Load &load);
    void showSolverCategory(const SimulationCase &simulationCase);
    void showResultCategory(const std::vector<ResultObject> &results);
    void showResult(const ResultObject &resultObject);

private:
    void clearAll();
    QWidget *resetDynamicArea(bool visible = true);

    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_selectionValue = nullptr;
    QLabel *m_typeValue = nullptr;
    QLabel *m_nameValue = nullptr;
    QLabel *m_radiusValue = nullptr;
    QLabel *m_lengthValue = nullptr;
    QLabel *m_widthValue = nullptr;
    QLabel *m_heightValue = nullptr;
    QLabel *m_centerValue = nullptr;
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
