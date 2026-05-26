#include "ui/ResultPostprocessPanel.h"

#include "result/ResultFieldMetadata.h"
#include "result/ResultObject.h"
#include "result/ResultProbe.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}

QString coverageText(const ResultObject &resultObject)
{
    const QString nodeText = resultObject.meshNodeCount > 0
        ? zh(u8"节点 %1/%2").arg(resultObject.matchedNodeCount).arg(resultObject.meshNodeCount)
        : zh(u8"节点 -");
    const QString elementText = resultObject.meshElementCount > 0
        ? zh(u8"单元 %1/%2").arg(resultObject.matchedElementCount).arg(resultObject.meshElementCount)
        : zh(u8"单元 -");
    return nodeText + zh(u8"，") + elementText;
}

QString fileStatusText(const ResultObject &resultObject)
{
    return resultObject.resultFilesComplete ? zh(u8"结果文件完整") : zh(u8"结果文件不完整");
}

QString nodeExtremeSummary(const ResultNodeExtreme &extreme)
{
    if (!extreme.valid) {
        return "-";
    }
    return zh(u8"%1：节点 %2，值 %3，坐标 (%4, %5, %6)")
        .arg(extreme.fieldName)
        .arg(extreme.nodeId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString elementExtremeSummary(const ResultElementExtreme &extreme)
{
    if (!extreme.valid) {
        return "-";
    }
    return zh(u8"%1：单元 %2，值 %3，中心 (%4, %5, %6)")
        .arg(extreme.fieldName)
        .arg(extreme.elementId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString markerSummary(const ResultExtremeMarker &marker)
{
    if (!marker.valid) {
        return "-";
    }
    return zh(u8"%1 %2，值 %3")
        .arg(marker.element ? zh(u8"单元") : zh(u8"节点"))
        .arg(marker.id)
        .arg(marker.value, 0, 'g', 6);
}

QString fieldUnitText(const ResultObject &resultObject)
{
    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const QString unit = ResultFieldMetadata::unitForField(fieldName);
    return zh(u8"当前场：") + fieldName + zh(u8"，单位：") + unit;
}

QString coordinateText(double x, double y, double z)
{
    return QString("(%1, %2, %3)").arg(x, 0, 'g', 6).arg(y, 0, 'g', 6).arg(z, 0, 'g', 6);
}

void configureRangeSpinBox(QDoubleSpinBox *spinBox)
{
    if (!spinBox) {
        return;
    }
    spinBox->setRange(-1.0e15, 1.0e15);
    spinBox->setDecimals(8);
    spinBox->setSingleStep(1.0);
}

QGroupBox *createSection(QVBoxLayout *layout, const QString &title, QWidget *parent, const QString &objectName)
{
    auto *group = new QGroupBox(title, parent);
    group->setObjectName(objectName);
    layout->addWidget(group);
    return group;
}

QLabel *createStatusLabel(const QString &text, QWidget *parent, const QString &objectName)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}

QLabel *createValueLabel(QWidget *parent, const QString &objectName)
{
    auto *label = createStatusLabel("-", parent, objectName);
    label->setMinimumWidth(110);
    return label;
}

void addKeyValueRow(QGridLayout *layout, int row, const QString &key, QLabel *value, QWidget *parent)
{
    auto *keyLabel = new QLabel(key, parent);
    keyLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    layout->addWidget(keyLabel, row, 0);
    layout->addWidget(value, row, 1);
}

QFormLayout *createFormLayout(QWidget *parent)
{
    auto *form = new QFormLayout(parent);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(5);
    return form;
}
}

ResultPostprocessPanel::ResultPostprocessPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    QGroupBox *resultGroup = createSection(layout, zh(u8"结果"), this, "result.section.identity");
    auto *resultLayout = new QVBoxLayout(resultGroup);
    m_resultNameLabel = createStatusLabel(zh(u8"未选择结果"), this, "result.name.label");
    resultLayout->addWidget(m_resultNameLabel);

    QGroupBox *displayGroup = createSection(layout, zh(u8"显示"), this, "result.section.display");
    auto *form = createFormLayout(displayGroup);
    m_fieldComboBox = new QComboBox(this);
    m_fieldComboBox->setObjectName("result.field.combo");
    m_fieldComboBox->addItems(QStringList{
        CalculiXResultFields::Ux,
        CalculiXResultFields::Uy,
        CalculiXResultFields::Uz,
        CalculiXResultFields::DisplacementMagnitude,
        CalculiXResultFields::VonMisesStress
    });
    form->addRow(zh(u8"结果场"), m_fieldComboBox);

    m_scaleSpinBox = new QDoubleSpinBox(this);
    m_scaleSpinBox->setObjectName("result.deformationScale.spin");
    m_scaleSpinBox->setRange(0.0, 1000000.0);
    m_scaleSpinBox->setDecimals(3);
    m_scaleSpinBox->setSingleStep(1.0);
    m_scaleSpinBox->setSuffix(" x");
    form->addRow(zh(u8"变形比例"), m_scaleSpinBox);

    m_meshEdgesCheckBox = new QCheckBox(zh(u8"显示网格边"), this);
    m_meshEdgesCheckBox->setObjectName("result.meshEdges.checkbox");
    form->addRow("", m_meshEdgesCheckBox);

    m_undeformedOverlayCheckBox = new QCheckBox(zh(u8"显示未变形轮廓"), this);
    m_undeformedOverlayCheckBox->setObjectName("result.undeformedOverlay.checkbox");
    form->addRow("", m_undeformedOverlayCheckBox);

    m_fieldUnitLabel = createStatusLabel(zh(u8"当前场：-，单位：-"), this, "result.fieldUnit.label");
    form->addRow(zh(u8"单位"), m_fieldUnitLabel);

    m_lockScalarRangeCheckBox = new QCheckBox(zh(u8"锁定色标范围"), this);
    m_lockScalarRangeCheckBox->setObjectName("result.scalarRange.lock");
    form->addRow("", m_lockScalarRangeCheckBox);

    m_scalarMinSpinBox = new QDoubleSpinBox(this);
    m_scalarMinSpinBox->setObjectName("result.scalarRange.min");
    configureRangeSpinBox(m_scalarMinSpinBox);
    form->addRow(zh(u8"色标下限"), m_scalarMinSpinBox);

    m_scalarMaxSpinBox = new QDoubleSpinBox(this);
    m_scalarMaxSpinBox->setObjectName("result.scalarRange.max");
    configureRangeSpinBox(m_scalarMaxSpinBox);
    form->addRow(zh(u8"色标上限"), m_scalarMaxSpinBox);

    m_scalarRangeLabel = createStatusLabel(zh(u8"色标范围：-"), this, "result.scalarRange.label");
    form->addRow(zh(u8"当前范围"), m_scalarRangeLabel);

    QGroupBox *probeGroup = createSection(layout, zh(u8"点选查询"), this, "result.section.probe");
    auto *probeLayout = new QGridLayout(probeGroup);
    probeLayout->setColumnStretch(1, 1);
    probeLayout->setHorizontalSpacing(10);
    probeLayout->setVerticalSpacing(5);
    m_probeHintLabel =
        createStatusLabel(zh(u8"点击结果模型后显示最近节点和拾取单元。"), this, "result.probe.label");
    probeLayout->addWidget(m_probeHintLabel, 0, 0, 1, 2);
    m_probeNodeIdValue = createValueLabel(this, "result.probe.nodeId");
    m_probeElementIdValue = createValueLabel(this, "result.probe.elementId");
    m_probeCoordinateValue = createValueLabel(this, "result.probe.coordinate");
    m_probeUxValue = createValueLabel(this, "result.probe.ux");
    m_probeUyValue = createValueLabel(this, "result.probe.uy");
    m_probeUzValue = createValueLabel(this, "result.probe.uz");
    m_probeMagnitudeValue = createValueLabel(this, "result.probe.magnitude");
    m_probeVonMisesValue = createValueLabel(this, "result.probe.vonMises");
    addKeyValueRow(probeLayout, 1, zh(u8"节点 ID"), m_probeNodeIdValue, this);
    addKeyValueRow(probeLayout, 2, zh(u8"单元 ID"), m_probeElementIdValue, this);
    addKeyValueRow(probeLayout, 3, zh(u8"坐标"), m_probeCoordinateValue, this);
    addKeyValueRow(probeLayout, 4, "Ux", m_probeUxValue, this);
    addKeyValueRow(probeLayout, 5, "Uy", m_probeUyValue, this);
    addKeyValueRow(probeLayout, 6, "Uz", m_probeUzValue, this);
    addKeyValueRow(probeLayout, 7, zh(u8"位移模"), m_probeMagnitudeValue, this);
    addKeyValueRow(probeLayout, 8, "Von Mises", m_probeVonMisesValue, this);

    QGroupBox *extremesGroup = createSection(layout, zh(u8"极值"), this, "result.section.extremes");
    auto *extremesLayout = new QGridLayout(extremesGroup);
    extremesLayout->setColumnStretch(1, 1);
    extremesLayout->setHorizontalSpacing(10);
    extremesLayout->setVerticalSpacing(5);
    m_currentMinValue = createValueLabel(this, "result.extreme.currentMin");
    m_currentMaxValue = createValueLabel(this, "result.extreme.currentMax");
    m_maxDisplacementValue = createValueLabel(this, "result.extreme.maxDisplacement");
    m_maxUxValue = createValueLabel(this, "result.extreme.maxUx");
    m_maxUyValue = createValueLabel(this, "result.extreme.maxUy");
    m_maxUzValue = createValueLabel(this, "result.extreme.maxUz");
    m_maxVonMisesValue = createValueLabel(this, "result.extreme.maxVonMises");
    addKeyValueRow(extremesLayout, 0, zh(u8"当前场最小"), m_currentMinValue, this);
    addKeyValueRow(extremesLayout, 1, zh(u8"当前场最大"), m_currentMaxValue, this);
    addKeyValueRow(extremesLayout, 2, zh(u8"最大位移模"), m_maxDisplacementValue, this);
    addKeyValueRow(extremesLayout, 3, "Max Ux", m_maxUxValue, this);
    addKeyValueRow(extremesLayout, 4, "Max Uy", m_maxUyValue, this);
    addKeyValueRow(extremesLayout, 5, "Max Uz", m_maxUzValue, this);
    addKeyValueRow(extremesLayout, 6, "Max Von Mises", m_maxVonMisesValue, this);

    QGroupBox *statusGroup = createSection(layout, zh(u8"状态"), this, "result.section.status");
    auto *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(5);
    m_coverageLabel = createStatusLabel(zh(u8"覆盖率：-"), this, "result.coverage.label");
    m_fileStatusLabel = createStatusLabel(zh(u8"文件：-"), this, "result.fileStatus.label");
    m_messagesLabel = createStatusLabel("-", this, "result.messages.label");
    statusLayout->addWidget(m_coverageLabel);
    statusLayout->addWidget(m_fileStatusLabel);
    statusLayout->addWidget(m_messagesLabel);

    QGroupBox *animationGroup = createSection(layout, zh(u8"动画"), this, "result.section.animation");
    auto *animationLayout = new QGridLayout(animationGroup);
    animationLayout->setHorizontalSpacing(6);
    animationLayout->setVerticalSpacing(6);
    m_animationSpeedSpinBox = new QDoubleSpinBox(this);
    m_animationSpeedSpinBox->setObjectName("result.animationSpeed.spin");
    m_animationSpeedSpinBox->setRange(0.1, 5.0);
    m_animationSpeedSpinBox->setDecimals(2);
    m_animationSpeedSpinBox->setSingleStep(0.25);
    m_animationSpeedSpinBox->setValue(1.0);
    m_animationSpeedSpinBox->setSuffix(" Hz");
    m_playAnimationButton = new QPushButton(zh(u8"播放"), this);
    m_playAnimationButton->setObjectName("result.animation.play");
    m_stopAnimationButton = new QPushButton(zh(u8"停止"), this);
    m_stopAnimationButton->setObjectName("result.animation.stop");
    m_animationFrameLabel = createStatusLabel(zh(u8"当前帧比例：-"), this, "result.animationFrame.label");
    animationLayout->addWidget(new QLabel(zh(u8"速度"), this), 0, 0);
    animationLayout->addWidget(m_animationSpeedSpinBox, 0, 1, 1, 2);
    animationLayout->addWidget(m_playAnimationButton, 1, 1);
    animationLayout->addWidget(m_stopAnimationButton, 1, 2);
    animationLayout->addWidget(m_animationFrameLabel, 2, 0, 1, 3);

    QGroupBox *exportGroup = createSection(layout, zh(u8"导出 / 历史"), this, "result.section.exportHistory");
    auto *exportLayout = new QGridLayout(exportGroup);
    exportLayout->setHorizontalSpacing(6);
    exportLayout->setVerticalSpacing(6);
    m_exportCsvButton = new QPushButton(zh(u8"导出 CSV"), this);
    m_exportCsvButton->setObjectName("result.export.csv");
    m_exportReportButton = new QPushButton(zh(u8"导出报告"), this);
    m_exportReportButton->setObjectName("result.export.report");
    m_exportScreenshotButton = new QPushButton(zh(u8"导出截图"), this);
    m_exportScreenshotButton->setObjectName("result.export.screenshot");
    m_openDirectoryButton = new QPushButton(zh(u8"打开目录"), this);
    m_openDirectoryButton->setObjectName("result.export.openDirectory");
    exportLayout->addWidget(m_exportCsvButton, 0, 0);
    exportLayout->addWidget(m_exportReportButton, 0, 1);
    exportLayout->addWidget(m_exportScreenshotButton, 1, 0);
    exportLayout->addWidget(m_openDirectoryButton, 1, 1);
    m_renameButton = new QPushButton(zh(u8"重命名"), this);
    m_renameButton->setObjectName("result.history.rename");
    m_deleteButton = new QPushButton(zh(u8"删除记录"), this);
    m_deleteButton->setObjectName("result.history.delete");
    exportLayout->addWidget(m_renameButton, 2, 0);
    exportLayout->addWidget(m_deleteButton, 2, 1);
    layout->addStretch();

    connect(m_fieldComboBox, &QComboBox::currentTextChanged, this, [this](const QString &fieldName) {
        if (!m_updating) {
            emit fieldChanged(fieldName);
        }
    });
    connect(m_scaleSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double scale) {
        if (!m_updating) {
            emit deformationScaleChanged(scale);
        }
    });
    connect(m_meshEdgesCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        if (!m_updating) {
            emit meshEdgesChanged(enabled);
        }
    });
    connect(m_undeformedOverlayCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        if (!m_updating) {
            emit undeformedOverlayChanged(enabled);
        }
    });
    connect(m_lockScalarRangeCheckBox, &QCheckBox::toggled, this, [this](bool locked) {
        m_scalarMinSpinBox->setEnabled(locked);
        m_scalarMaxSpinBox->setEnabled(locked);
        if (!m_updating) {
            emit scalarRangeLockChanged(locked);
        }
    });
    connect(m_scalarMinSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double minimum) {
        if (!m_updating) {
            emit scalarRangeChanged(minimum, m_scalarMaxSpinBox->value());
        }
    });
    connect(m_scalarMaxSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double maximum) {
        if (!m_updating) {
            emit scalarRangeChanged(m_scalarMinSpinBox->value(), maximum);
        }
    });
    connect(m_playAnimationButton, &QPushButton::clicked, this, [this]() {
        emit animationPlayRequested(m_animationSpeedSpinBox->value());
    });
    connect(m_stopAnimationButton, &QPushButton::clicked, this, &ResultPostprocessPanel::animationStopRequested);
    connect(m_exportCsvButton, &QPushButton::clicked, this, &ResultPostprocessPanel::exportCsvRequested);
    connect(m_exportReportButton, &QPushButton::clicked, this, &ResultPostprocessPanel::exportReportRequested);
    connect(m_exportScreenshotButton, &QPushButton::clicked, this, &ResultPostprocessPanel::exportScreenshotRequested);
    connect(m_openDirectoryButton, &QPushButton::clicked, this, &ResultPostprocessPanel::openResultDirectoryRequested);
    connect(m_renameButton, &QPushButton::clicked, this, &ResultPostprocessPanel::renameResultRequested);
    connect(m_deleteButton, &QPushButton::clicked, this, &ResultPostprocessPanel::deleteResultRequested);

    setEnabledForResult(false);
}

void ResultPostprocessPanel::setResult(const ResultObject *resultObject)
{
    m_updating = true;
    if (!resultObject) {
        m_resultNameLabel->setText(zh(u8"未选择结果"));
        m_scalarRangeLabel->setText(zh(u8"色标范围：-"));
        m_coverageLabel->setText(zh(u8"覆盖率：-"));
        m_fileStatusLabel->setText(zh(u8"文件：-"));
        m_messagesLabel->setText("-");
        setProbe(ResultProbe{});
        m_currentMinValue->setText("-");
        m_currentMaxValue->setText("-");
        m_maxDisplacementValue->setText("-");
        m_maxUxValue->setText("-");
        m_maxUyValue->setText("-");
        m_maxUzValue->setText("-");
        m_maxVonMisesValue->setText("-");
        m_fieldUnitLabel->setText(zh(u8"当前场：-，单位：-"));
        m_animationFrameLabel->setText(zh(u8"当前帧比例：-"));
        setEnabledForResult(false);
        m_updating = false;
        return;
    }

    m_resultNameLabel->setText(resultObject->name);
    const QString fieldName = resultObject->displayFieldName.isEmpty()
        ? resultObject->primaryFieldName
        : resultObject->displayFieldName;
    const int fieldIndex = m_fieldComboBox->findText(fieldName);
    if (fieldIndex >= 0) {
        m_fieldComboBox->setCurrentIndex(fieldIndex);
    }
    m_scaleSpinBox->setValue(resultObject->deformationScale);
    m_lockScalarRangeCheckBox->setChecked(resultObject->scalarRangeLocked);
    m_scalarMinSpinBox->setValue(resultObject->lockedScalarMin);
    m_scalarMaxSpinBox->setValue(resultObject->lockedScalarMax);
    m_animationFrameLabel->setText(zh(u8"当前帧比例：%1 x").arg(resultObject->deformationScale, 0, 'g', 6));
    m_meshEdgesCheckBox->setChecked(resultObject->showMeshEdges);
    m_undeformedOverlayCheckBox->setChecked(resultObject->showUndeformedOverlay);
    setStatusText(resultObject);
    setEnabledForResult(true);
    m_updating = false;
}

void ResultPostprocessPanel::setEnabledForResult(bool enabled)
{
    m_fieldComboBox->setEnabled(enabled);
    m_scaleSpinBox->setEnabled(enabled);
    m_meshEdgesCheckBox->setEnabled(enabled);
    m_undeformedOverlayCheckBox->setEnabled(enabled);
    m_lockScalarRangeCheckBox->setEnabled(enabled);
    const bool rangeControlsEnabled = enabled && m_lockScalarRangeCheckBox->isChecked();
    m_scalarMinSpinBox->setEnabled(rangeControlsEnabled);
    m_scalarMaxSpinBox->setEnabled(rangeControlsEnabled);
    m_animationSpeedSpinBox->setEnabled(enabled);
    m_playAnimationButton->setEnabled(enabled);
    m_stopAnimationButton->setEnabled(enabled);
    m_exportCsvButton->setEnabled(enabled);
    m_exportReportButton->setEnabled(enabled);
    m_exportScreenshotButton->setEnabled(enabled);
    m_openDirectoryButton->setEnabled(enabled);
    m_renameButton->setEnabled(enabled);
    m_deleteButton->setEnabled(enabled);
}

void ResultPostprocessPanel::setStatusText(const ResultObject *resultObject)
{
    if (!resultObject) {
        return;
    }

    const QString rangeMode = resultObject->scalarRangeLocked ? zh(u8"锁定") : zh(u8"自动");
    const QString fieldName = resultObject->displayFieldName.isEmpty()
        ? resultObject->primaryFieldName
        : resultObject->displayFieldName;
    const QString unit = ResultFieldMetadata::unitForField(fieldName);
    const double rangeMinimum = resultObject->scalarRangeLocked ? resultObject->lockedScalarMin : resultObject->scalarMin;
    const double rangeMaximum = resultObject->scalarRangeLocked ? resultObject->lockedScalarMax : resultObject->scalarMax;
    m_scalarRangeLabel->setText(zh(u8"色标范围（%1）：%2 到 %3 %4")
        .arg(rangeMode)
        .arg(rangeMinimum, 0, 'g', 6)
        .arg(rangeMaximum, 0, 'g', 6)
        .arg(unit));
    m_fieldUnitLabel->setText(fieldUnitText(*resultObject));
    m_coverageLabel->setText(zh(u8"覆盖率：") + coverageText(*resultObject));
    m_currentMinValue->setText(markerSummary(resultObject->extrema.selectedMinimumMarker));
    m_currentMaxValue->setText(markerSummary(resultObject->extrema.selectedMaximumMarker));
    m_maxDisplacementValue->setText(nodeExtremeSummary(resultObject->extrema.maxDisplacementMagnitude));
    m_maxUxValue->setText(nodeExtremeSummary(resultObject->extrema.maxUx));
    m_maxUyValue->setText(nodeExtremeSummary(resultObject->extrema.maxUy));
    m_maxUzValue->setText(nodeExtremeSummary(resultObject->extrema.maxUz));
    m_maxVonMisesValue->setText(elementExtremeSummary(resultObject->extrema.maxVonMisesStress));
    m_fileStatusLabel->setText(zh(u8"文件：") + fileStatusText(*resultObject));
    m_messagesLabel->setText(resultObject->checkMessages.isEmpty()
        ? zh(u8"检查：通过")
        : zh(u8"检查：") + resultObject->checkMessages.join("; "));
    setProbe(ResultProbe{});
}

void ResultPostprocessPanel::setProbe(const ResultProbe &probe)
{
    if (!probe.valid) {
        m_probeHintLabel->setText(zh(u8"点击结果模型后显示最近节点和拾取单元。"));
        m_probeNodeIdValue->setText("-");
        m_probeElementIdValue->setText("-");
        m_probeCoordinateValue->setText("-");
        m_probeUxValue->setText("-");
        m_probeUyValue->setText("-");
        m_probeUzValue->setText("-");
        m_probeMagnitudeValue->setText("-");
        m_probeVonMisesValue->setText("-");
        return;
    }

    m_probeHintLabel->setText(zh(u8"已选中结果模型上的查询点。"));
    m_probeNodeIdValue->setText(QString::number(probe.nodeId));
    m_probeElementIdValue->setText(QString::number(probe.elementId));
    m_probeCoordinateValue->setText(coordinateText(probe.x, probe.y, probe.z));
    m_probeUxValue->setText(QString::number(probe.ux, 'g', 6));
    m_probeUyValue->setText(QString::number(probe.uy, 'g', 6));
    m_probeUzValue->setText(QString::number(probe.uz, 'g', 6));
    m_probeMagnitudeValue->setText(QString::number(probe.displacementMagnitude, 'g', 6));
    m_probeVonMisesValue->setText(QString::number(probe.vonMisesStress, 'g', 6));
}
