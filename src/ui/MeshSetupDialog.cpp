#include "MeshSetupDialog.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <cmath>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

void configureSizeSpinBox(QDoubleSpinBox *spinBox)
{
    spinBox->setRange(0.0, 1.0e9);
    spinBox->setDecimals(6);
    spinBox->setSingleStep(1.0);
    spinBox->setSpecialValueText(zh(u8"禁用"));
}

double activeLocalMeshSize(const FaceGroup &faceGroup)
{
    return faceGroup.localMeshEnabled && faceGroup.localMeshSize > 0.0 ? faceGroup.localMeshSize : 0.0;
}

QString localMeshStatusText(const FaceGroup &faceGroup)
{
    if (faceGroup.faceIndices.empty()) {
        return zh(u8"无面，无法导出");
    }
    return activeLocalMeshSize(faceGroup) > 0.0 ? zh(u8"启用") : zh(u8"禁用");
}

QTableWidgetItem *readOnlyItem(const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
}

const FaceGroup *findFaceGroupById(const std::vector<FaceGroup> &faceGroups, const QString &id)
{
    for (const FaceGroup &faceGroup : faceGroups) {
        if (faceGroup.id == id) {
            return &faceGroup;
        }
    }
    return nullptr;
}
}

MeshSetupDialog::MeshSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(zh(u8"网格设置"));
    setupUi();
}

void MeshSetupDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *globalGroup = new QGroupBox(zh(u8"全局网格"), this);
    auto *form = new QFormLayout(globalGroup);

    m_elementTypeCombo = new QComboBox(this);
    m_elementTypeCombo->addItem(zh(u8"Tet4 - 线性四面体"), toString(MeshElementType::Tetra4));
    m_elementTypeCombo->addItem(zh(u8"Tet10 - 二次四面体"), toString(MeshElementType::Tetra10));
    form->addRow(zh(u8"单元类型:"), m_elementTypeCombo);
    connect(m_elementTypeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateSizePreview();
    });

    m_algorithmCombo = new QComboBox(this);
    m_algorithmCombo->addItem(zh(u8"默认"), toString(GmshMeshAlgorithm3D::Default));
    m_algorithmCombo->addItem(zh(u8"Delaunay"), toString(GmshMeshAlgorithm3D::Delaunay));
    m_algorithmCombo->addItem(zh(u8"Frontal-Delaunay"), toString(GmshMeshAlgorithm3D::Frontal));
    m_algorithmCombo->addItem(zh(u8"HXT"), toString(GmshMeshAlgorithm3D::HXT));
    form->addRow(zh(u8"3D 算法:"), m_algorithmCombo);
    connect(m_algorithmCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
        updateSizePreview();
    });

    m_autoSizeCheckBox = new QCheckBox(this);
    form->addRow(zh(u8"自动尺寸:"), m_autoSizeCheckBox);

    m_minimumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_minimumSizeSpin);
    form->addRow(zh(u8"最小尺寸:"), m_minimumSizeSpin);

    m_maximumSizeSpin = new QDoubleSpinBox(this);
    configureSizeSpinBox(m_maximumSizeSpin);
    form->addRow(zh(u8"最大尺寸:"), m_maximumSizeSpin);

    m_meshSizeUnitCombo = new QComboBox(this);
    m_meshSizeUnitCombo->addItems({"mm", "m"});
    form->addRow(zh(u8"尺寸单位:"), m_meshSizeUnitCombo);

    connect(m_autoSizeCheckBox, &QCheckBox::toggled, this, [this]() {
        updateSizeControlState();
        updateSizePreview();
    });
    connect(m_minimumSizeSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this]() {
        updateSizePreview();
    });
    connect(m_maximumSizeSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this]() {
        updateSizePreview();
    });
    connect(m_meshSizeUnitCombo, &QComboBox::currentTextChanged, this, [this](const QString &unit) {
        const QString suffix = " " + unit;
        m_minimumSizeSpin->setSuffix(suffix);
        m_maximumSizeSpin->setSuffix(suffix);
        updateSizePreview();
    });

    mainLayout->addWidget(globalGroup);

    m_sizePreviewLabel = new QLabel(this);
    m_sizePreviewLabel->setWordWrap(true);
    m_sizePreviewLabel->setObjectName("mesh.setup.sizePreview");
    m_sizePreviewLabel->setStyleSheet(
        "QLabel {"
        "  color: #1f2937;"
        "  background: #f8fafc;"
        "  border: 1px solid #d1d5db;"
        "  border-radius: 4px;"
        "  padding: 6px 8px;"
        "}"
    );
    mainLayout->addWidget(m_sizePreviewLabel);

    auto *localGroup = new QGroupBox(zh(u8"局部加密"), this);
    auto *localLayout = new QVBoxLayout(localGroup);

    m_localMeshHintLabel = new QLabel(
        zh(u8"局部加密尺寸作用在选定面组的几何面上，数值越小网格越密，0 表示禁用。全局最小/最大尺寸仍可能限制最终效果，修改后需要重新生成网格。"),
        this
    );
    m_localMeshHintLabel->setWordWrap(true);
    localLayout->addWidget(m_localMeshHintLabel);

    m_localMeshTable = new QTableWidget(this);
    m_localMeshTable->setColumnCount(4);
    m_localMeshTable->setHorizontalHeaderLabels({
        zh(u8"面组"),
        zh(u8"面数量"),
        zh(u8"局部尺寸"),
        zh(u8"状态")
    });
    m_localMeshTable->horizontalHeader()->setStretchLastSection(true);
    m_localMeshTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_localMeshTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_localMeshTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_localMeshTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_localMeshTable->verticalHeader()->setVisible(false);
    m_localMeshTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_localMeshTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_localMeshTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_localMeshTable->setMinimumHeight(150);
    connect(m_localMeshTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        updateLocalMeshButtons();
    });
    localLayout->addWidget(m_localMeshTable);

    auto *localButtonLayout = new QHBoxLayout;
    localButtonLayout->addStretch();
    m_editLocalMeshButton = new QPushButton(zh(u8"编辑局部尺寸"), this);
    m_disableLocalMeshButton = new QPushButton(zh(u8"禁用局部加密"), this);
    connect(m_editLocalMeshButton, &QPushButton::clicked, this, [this]() {
        editSelectedLocalMeshSize();
    });
    connect(m_disableLocalMeshButton, &QPushButton::clicked, this, [this]() {
        disableSelectedLocalMeshSize();
    });
    localButtonLayout->addWidget(m_editLocalMeshButton);
    localButtonLayout->addWidget(m_disableLocalMeshButton);
    localLayout->addLayout(localButtonLayout);

    mainLayout->addWidget(localGroup);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    buttonBox->button(QDialogButtonBox::Ok)->setText(zh(u8"确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(zh(u8"取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (!m_autoSizeCheckBox->isChecked()
            && m_minimumSizeSpin->value() > 0.0
            && m_maximumSizeSpin->value() > 0.0
            && m_minimumSizeSpin->value() > m_maximumSizeSpin->value()) {
            QMessageBox::warning(
                this,
                zh(u8"校验"),
                zh(u8"最小网格尺寸不能大于最大网格尺寸。")
            );
            return;
        }
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    refreshLocalMeshTable();
}

MeshSetup MeshSetupDialog::meshSetup() const
{
    MeshSetup setup = m_currentSetup;
    setup.elementType = selectedElementType();
    setup.algorithm = selectedAlgorithm();
    setup.autoSize = m_autoSizeCheckBox->isChecked();
    setup.minimumSize = setup.autoSize ? 0.0 : m_minimumSizeSpin->value();
    setup.maximumSize = setup.autoSize ? 0.0 : m_maximumSizeSpin->value();
    setup.meshSizeUnit = m_meshSizeUnitCombo->currentText().trimmed().isEmpty()
        ? QStringLiteral("mm")
        : m_meshSizeUnitCombo->currentText().trimmed();
    return setup;
}

void MeshSetupDialog::setMeshSetup(const MeshSetup &meshSetup)
{
    m_currentSetup = meshSetup;
    setElementType(meshSetup.elementType);
    setAlgorithm(meshSetup.algorithm);
    m_autoSizeCheckBox->setChecked(meshSetup.autoSize);
    m_minimumSizeSpin->setValue(meshSetup.minimumSize);
    m_maximumSizeSpin->setValue(meshSetup.maximumSize);
    const QString meshSizeUnit = meshSetup.meshSizeUnit.trimmed().isEmpty()
        ? QStringLiteral("mm")
        : meshSetup.meshSizeUnit.trimmed();
    const int unitIndex = m_meshSizeUnitCombo->findText(meshSizeUnit);
    m_meshSizeUnitCombo->setCurrentIndex(unitIndex >= 0 ? unitIndex : 0);
    updateSizeControlState();
    updateSizePreview();
}

void MeshSetupDialog::setOptions(const MeshSetupDialogOptions &options)
{
    m_options = options;
    m_originalFaceGroups = options.faceGroups;
    m_pendingFaceGroups = options.faceGroups;
    if (!options.geometryName.trimmed().isEmpty()) {
        setWindowTitle(zh(u8"网格设置 - ") + options.geometryName.trimmed());
    }
    refreshLocalMeshTable();
}

MeshSetupDialogResult MeshSetupDialog::dialogResult() const
{
    MeshSetupDialogResult result;
    result.meshSetup = meshSetup();

    for (const FaceGroup &pendingFaceGroup : m_pendingFaceGroups) {
        const FaceGroup *originalFaceGroup = findFaceGroupById(m_originalFaceGroups, pendingFaceGroup.id);
        const double oldSize = originalFaceGroup ? activeLocalMeshSize(*originalFaceGroup) : 0.0;
        const double newSize = activeLocalMeshSize(pendingFaceGroup);
        if (std::abs(oldSize - newSize) <= 1.0e-12) {
            continue;
        }

        LocalMeshSizeChange change;
        change.faceGroupId = pendingFaceGroup.id;
        change.localMeshSize = newSize;
        result.localMeshSizeChanges.push_back(change);
    }
    return result;
}

void MeshSetupDialog::updateSizeControlState()
{
    const bool manual = !m_autoSizeCheckBox->isChecked();
    m_minimumSizeSpin->setEnabled(manual);
    m_maximumSizeSpin->setEnabled(manual);
}

void MeshSetupDialog::updateSizePreview()
{
    if (!m_sizePreviewLabel) {
        return;
    }

    const QString elementType = m_elementTypeCombo
        ? m_elementTypeCombo->currentText()
        : displayName(MeshElementType::Tetra4);
    const QString algorithm = m_algorithmCombo
        ? m_algorithmCombo->currentText()
        : displayName(GmshMeshAlgorithm3D::Default);
    const QString unit = m_meshSizeUnitCombo ? m_meshSizeUnitCombo->currentText() : QStringLiteral("mm");
    if (m_autoSizeCheckBox->isChecked()) {
        m_sizePreviewLabel->setText(
            zh(u8"当前设置：%1，3D 算法 %2，自动尺寸。网格内部长度单位为 %3。Gmsh 会根据几何包围盒和曲率估算全局网格尺寸。")
                .arg(elementType)
                .arg(algorithm)
                .arg(unit)
        );
        return;
    }

    m_sizePreviewLabel->setText(
        zh(u8"当前设置：%1，3D 算法 %2，手动尺寸。最小尺寸 %3 %5，最大尺寸 %4 %5。局部加密尺寸可在下方按面组覆盖，修改后需要重新生成网格。")
            .arg(elementType)
            .arg(algorithm)
            .arg(m_minimumSizeSpin->value(), 0, 'g', 8)
            .arg(m_maximumSizeSpin->value(), 0, 'g', 8)
            .arg(unit)
    );
}

void MeshSetupDialog::setElementType(MeshElementType elementType)
{
    const int index = m_elementTypeCombo->findData(toString(elementType));
    m_elementTypeCombo->setCurrentIndex(index >= 0 ? index : 0);
    updateSizePreview();
}

MeshElementType MeshSetupDialog::selectedElementType() const
{
    const QString value = m_elementTypeCombo->currentData().toString();
    if (value == toString(MeshElementType::Tetra10)) {
        return MeshElementType::Tetra10;
    }
    return MeshElementType::Tetra4;
}

void MeshSetupDialog::setAlgorithm(GmshMeshAlgorithm3D algorithm)
{
    const int index = m_algorithmCombo->findData(toString(algorithm));
    m_algorithmCombo->setCurrentIndex(index >= 0 ? index : 0);
    updateSizePreview();
}

GmshMeshAlgorithm3D MeshSetupDialog::selectedAlgorithm() const
{
    return gmshMeshAlgorithm3DFromString(m_algorithmCombo->currentData().toString());
}

void MeshSetupDialog::refreshLocalMeshTable()
{
    if (!m_localMeshTable) {
        return;
    }

    m_localMeshTable->setRowCount(static_cast<int>(m_pendingFaceGroups.size()));
    for (int row = 0; row < static_cast<int>(m_pendingFaceGroups.size()); ++row) {
        const FaceGroup &faceGroup = m_pendingFaceGroups.at(row);
        auto *nameItem = readOnlyItem(FaceGroups::displayName(faceGroup));
        nameItem->setData(Qt::UserRole, faceGroup.id);
        m_localMeshTable->setItem(row, 0, nameItem);
        m_localMeshTable->setItem(
            row,
            1,
            readOnlyItem(QString::number(static_cast<int>(faceGroup.faceIndices.size())))
        );
        const double size = activeLocalMeshSize(faceGroup);
        m_localMeshTable->setItem(
            row,
            2,
            readOnlyItem(size > 0.0 ? QString::number(size, 'g', 12) : zh(u8"0（禁用）"))
        );
        m_localMeshTable->setItem(row, 3, readOnlyItem(localMeshStatusText(faceGroup)));
    }
    updateLocalMeshButtons();
}

void MeshSetupDialog::updateLocalMeshButtons()
{
    const int row = selectedLocalMeshRow();
    const bool hasSelection = row >= 0;
    m_editLocalMeshButton->setEnabled(hasSelection);
    m_disableLocalMeshButton->setEnabled(hasSelection);
}

void MeshSetupDialog::editSelectedLocalMeshSize()
{
    const int row = selectedLocalMeshRow();
    FaceGroup *faceGroup = pendingFaceGroupForRow(row);
    if (!faceGroup) {
        return;
    }

    bool accepted = false;
    const double value = QInputDialog::getDouble(
        this,
        zh(u8"编辑局部网格尺寸"),
        zh(u8"局部加密尺寸（数值越小网格越密，0 表示禁用，重新生成网格后生效）"),
        activeLocalMeshSize(*faceGroup),
        0.0,
        1.0e9,
        6,
        &accepted
    );
    if (!accepted) {
        return;
    }

    faceGroup->localMeshSize = value > 0.0 ? value : 0.0;
    faceGroup->localMeshEnabled = faceGroup->localMeshSize > 0.0;
    refreshLocalMeshTable();
    if (row >= 0 && row < m_localMeshTable->rowCount()) {
        m_localMeshTable->selectRow(row);
    }
}

void MeshSetupDialog::disableSelectedLocalMeshSize()
{
    const int row = selectedLocalMeshRow();
    FaceGroup *faceGroup = pendingFaceGroupForRow(row);
    if (!faceGroup) {
        return;
    }

    faceGroup->localMeshSize = 0.0;
    faceGroup->localMeshEnabled = false;
    refreshLocalMeshTable();
    if (row >= 0 && row < m_localMeshTable->rowCount()) {
        m_localMeshTable->selectRow(row);
    }
}

int MeshSetupDialog::selectedLocalMeshRow() const
{
    if (!m_localMeshTable) {
        return -1;
    }
    const QList<QTableWidgetItem *> selectedItems = m_localMeshTable->selectedItems();
    return selectedItems.isEmpty() ? -1 : selectedItems.first()->row();
}

FaceGroup *MeshSetupDialog::pendingFaceGroupForRow(int row)
{
    if (row < 0 || row >= static_cast<int>(m_pendingFaceGroups.size())) {
        return nullptr;
    }
    return &m_pendingFaceGroups[static_cast<std::size_t>(row)];
}

std::optional<MeshSetup> MeshSetupDialog::editMeshSetup(
    QWidget *parent,
    const MeshSetup &current
)
{
    MeshSetupDialog dialog(parent);
    dialog.setMeshSetup(current);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.meshSetup();
    }
    return std::nullopt;
}

std::optional<MeshSetupDialogResult> MeshSetupDialog::editMeshSetup(
    QWidget *parent,
    const MeshSetup &current,
    const MeshSetupDialogOptions &options
)
{
    MeshSetupDialog dialog(parent);
    dialog.setOptions(options);
    dialog.setMeshSetup(current);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.dialogResult();
    }
    return std::nullopt;
}
