#pragma once

#include "solver/BoundaryCondition.h"

#include <QDialog>
#include <QMap>
#include <QStringList>

#include <optional>
#include <vector>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;

struct BoundaryConditionDialogOptions
{
    QStringList geometryNames;
    QMap<QString, QStringList> faceGroupsByGeometry;
    QStringList materialIds;
    std::vector<BoundaryConditionType> allowedTypes;
    QString defaultGeometryName;
    QString defaultFaceGroupId;
};

class BoundaryConditionDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BoundaryConditionDialog(
        BoundaryConditionDialogOptions options = {},
        QWidget *parent = nullptr
    );

    BoundaryCondition boundaryCondition() const;
    void setBoundaryCondition(const BoundaryCondition &bc);

    static std::optional<BoundaryCondition> createBoundaryCondition(
        QWidget *parent,
        BoundaryConditionDialogOptions options = {}
    );
    static std::optional<BoundaryCondition> editBoundaryCondition(
        QWidget *parent,
        const BoundaryCondition &existing,
        BoundaryConditionDialogOptions options = {}
    );

private:
    void setupUi();
    void updateFaceGroupItems(const QString &geometryName);
    void updateDisplacementEditors();
    void setComboCurrentText(QComboBox *combo, const QString &text);
    QString selectedFaceGroupId() const;

    BoundaryConditionDialogOptions m_options;
    QMap<QString, QString> m_faceGroupNamesById;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_typeCombo = nullptr;
    QComboBox *m_geometryNameCombo = nullptr;
    QComboBox *m_faceGroupNameCombo = nullptr;
    QComboBox *m_materialIdCombo = nullptr;
    QCheckBox *m_uxCheck = nullptr;
    QCheckBox *m_uyCheck = nullptr;
    QCheckBox *m_uzCheck = nullptr;
    QDoubleSpinBox *m_uxSpin = nullptr;
    QDoubleSpinBox *m_uySpin = nullptr;
    QDoubleSpinBox *m_uzSpin = nullptr;
    QComboBox *m_displacementUnitCombo = nullptr;
};
