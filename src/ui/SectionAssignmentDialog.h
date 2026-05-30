#pragma once

#include "geometry/GeometryObject.h"
#include "mesh/MeshObject.h"
#include "solver/Material.h"
#include "solver/SectionAssignment.h"

#include <QDialog>
#include <QVector>

#include <optional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

struct SectionAssignmentDialogOptions
{
    QVector<Material> solidMaterials;
    QVector<GeometryObject> geometries;
    QVector<MeshObject> meshes;
    QString projectRootPath;
    QString defaultGeometryName;
    QString defaultMeshName;
};

class SectionAssignmentDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SectionAssignmentDialog(
        SectionAssignmentDialogOptions options = {},
        QWidget *parent = nullptr
    );

    SectionAssignment sectionAssignment() const;
    void setSectionAssignment(const SectionAssignment &sectionAssignment);

    static std::optional<SectionAssignment> createSectionAssignment(
        QWidget *parent,
        SectionAssignmentDialogOptions options = {}
    );
    static std::optional<SectionAssignment> editSectionAssignment(
        QWidget *parent,
        const SectionAssignment &existing,
        SectionAssignmentDialogOptions options = {}
    );

private:
    void setupUi();
    void refreshElementSetOptionsForSelectedMesh();
    void setComboCurrentData(QComboBox *combo, const QString &value);
    QString selectedData(const QComboBox *combo) const;
    QString generatedId() const;

    SectionAssignmentDialogOptions m_options;
    QString m_id;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_materialCombo = nullptr;
    QComboBox *m_geometryCombo = nullptr;
    QComboBox *m_meshCombo = nullptr;
    QComboBox *m_elementSetCombo = nullptr;
    QLabel *m_elementSetHintLabel = nullptr;
    QCheckBox *m_enabledCheck = nullptr;
};
