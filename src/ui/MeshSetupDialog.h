#pragma once

#include "mesh/MeshSetup.h"

#include <QDialog>

#include <optional>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;

class MeshSetupDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit MeshSetupDialog(QWidget *parent = nullptr);

    MeshSetup meshSetup() const;
    void setMeshSetup(const MeshSetup &meshSetup);

    static std::optional<MeshSetup> editMeshSetup(
        QWidget *parent,
        const MeshSetup &current
    );

private:
    void setupUi();
    void updateSizeControlState();
    void setElementType(MeshElementType elementType);
    MeshElementType selectedElementType() const;

    MeshSetup m_currentSetup;
    QComboBox *m_elementTypeCombo = nullptr;
    QCheckBox *m_autoSizeCheckBox = nullptr;
    QDoubleSpinBox *m_minimumSizeSpin = nullptr;
    QDoubleSpinBox *m_maximumSizeSpin = nullptr;
};
