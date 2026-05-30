#pragma once

#include "geometry/FaceGroup.h"
#include "mesh/MeshSetup.h"

#include <QDialog>
#include <QString>

#include <optional>
#include <vector>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTableWidget;

struct LocalMeshSizeChange
{
    QString faceGroupId;
    double localMeshSize = 0.0;
};

struct MeshSetupDialogOptions
{
    QString geometryName;
    std::vector<FaceGroup> faceGroups;
};

struct MeshSetupDialogResult
{
    MeshSetup meshSetup;
    std::vector<LocalMeshSizeChange> localMeshSizeChanges;
};

class MeshSetupDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit MeshSetupDialog(QWidget *parent = nullptr);

    MeshSetup meshSetup() const;
    void setMeshSetup(const MeshSetup &meshSetup);
    void setOptions(const MeshSetupDialogOptions &options);
    MeshSetupDialogResult dialogResult() const;

    static std::optional<MeshSetup> editMeshSetup(
        QWidget *parent,
        const MeshSetup &current
    );
    static std::optional<MeshSetupDialogResult> editMeshSetup(
        QWidget *parent,
        const MeshSetup &current,
        const MeshSetupDialogOptions &options
    );

private:
    void setupUi();
    void updateSizeControlState();
    void updateSizePreview();
    void setElementType(MeshElementType elementType);
    MeshElementType selectedElementType() const;
    void refreshLocalMeshTable();
    void updateLocalMeshButtons();
    void editSelectedLocalMeshSize();
    void disableSelectedLocalMeshSize();
    int selectedLocalMeshRow() const;
    FaceGroup *pendingFaceGroupForRow(int row);

    MeshSetup m_currentSetup;
    MeshSetupDialogOptions m_options;
    std::vector<FaceGroup> m_originalFaceGroups;
    std::vector<FaceGroup> m_pendingFaceGroups;
    QComboBox *m_elementTypeCombo = nullptr;
    QCheckBox *m_autoSizeCheckBox = nullptr;
    QDoubleSpinBox *m_minimumSizeSpin = nullptr;
    QDoubleSpinBox *m_maximumSizeSpin = nullptr;
    QLabel *m_sizePreviewLabel = nullptr;
    QLabel *m_localMeshHintLabel = nullptr;
    QTableWidget *m_localMeshTable = nullptr;
    QPushButton *m_editLocalMeshButton = nullptr;
    QPushButton *m_disableLocalMeshButton = nullptr;
};
