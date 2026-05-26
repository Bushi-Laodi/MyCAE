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
QString coverageText(const ResultObject &resultObject)
{
    const QString nodeText = resultObject.meshNodeCount > 0
        ? QString("Nodes %1/%2").arg(resultObject.matchedNodeCount).arg(resultObject.meshNodeCount)
        : QString("Nodes -");
    const QString elementText = resultObject.meshElementCount > 0
        ? QString("Elements %1/%2").arg(resultObject.matchedElementCount).arg(resultObject.meshElementCount)
        : QString("Elements -");
    return nodeText + ", " + elementText;
}

QString fileStatusText(const ResultObject &resultObject)
{
    return resultObject.resultFilesComplete ? "Result files complete" : "Result files incomplete";
}

QString nodeExtremeText(const ResultNodeExtreme &extreme, const QString &label)
{
    if (!extreme.valid) {
        return label + ": -";
    }
    return QString("%1 %2: node %3, value %4, (%5, %6, %7)")
        .arg(label)
        .arg(extreme.fieldName)
        .arg(extreme.nodeId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString elementExtremeText(const ResultElementExtreme &extreme, const QString &label)
{
    if (!extreme.valid) {
        return label + ": -";
    }
    return QString("%1 %2: element %3, value %4, centroid (%5, %6, %7)")
        .arg(label)
        .arg(extreme.fieldName)
        .arg(extreme.elementId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString extremaText(const ResultObject &resultObject)
{
    QStringList lines;
    if (resultObject.extrema.selectedMinimumMarker.valid) {
        const ResultExtremeMarker &marker = resultObject.extrema.selectedMinimumMarker;
        const QString idLabel = marker.element ? "element" : "node";
        lines.append(QString("Current min: %1 %2, value %3")
            .arg(idLabel)
            .arg(marker.id)
            .arg(marker.value, 0, 'g', 6));
    }
    if (resultObject.extrema.selectedMaximumMarker.valid) {
        const ResultExtremeMarker &marker = resultObject.extrema.selectedMaximumMarker;
        const QString idLabel = marker.element ? "element" : "node";
        lines.append(QString("Current max: %1 %2, value %3")
            .arg(idLabel)
            .arg(marker.id)
            .arg(marker.value, 0, 'g', 6));
    }
    lines.append(nodeExtremeText(resultObject.extrema.maxDisplacementMagnitude, "Max"));
    lines.append(nodeExtremeText(resultObject.extrema.maxUx, "Max"));
    lines.append(nodeExtremeText(resultObject.extrema.maxUy, "Max"));
    lines.append(nodeExtremeText(resultObject.extrema.maxUz, "Max"));
    lines.append(elementExtremeText(resultObject.extrema.maxVonMisesStress, "Max"));
    return lines.join("\n");
}

QString fieldUnitText(const ResultObject &resultObject)
{
    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const QString unit = ResultFieldMetadata::unitForField(fieldName);
    return "Current field: " + fieldName + ", unit: " + unit;
}

QString probeText(const ResultProbe &probe)
{
    if (!probe.valid) {
        return "Probe: click the result model to query nearest node and picked element.";
    }
    return QString("Probe: node %1, element %2\n"
                   "Coordinate: (%3, %4, %5)\n"
                   "Ux: %6, Uy: %7, Uz: %8\n"
                   "Displacement Magnitude: %9\n"
                   "Von Mises Stress: %10")
        .arg(probe.nodeId)
        .arg(probe.elementId)
        .arg(probe.x, 0, 'g', 6)
        .arg(probe.y, 0, 'g', 6)
        .arg(probe.z, 0, 'g', 6)
        .arg(probe.ux, 0, 'g', 6)
        .arg(probe.uy, 0, 'g', 6)
        .arg(probe.uz, 0, 'g', 6)
        .arg(probe.displacementMagnitude, 0, 'g', 6)
        .arg(probe.vonMisesStress, 0, 'g', 6);
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

    QGroupBox *resultGroup = createSection(layout, "Result", this, "result.section.identity");
    auto *resultLayout = new QVBoxLayout(resultGroup);
    m_resultNameLabel = createStatusLabel("No result selected", this, "result.name.label");
    resultLayout->addWidget(m_resultNameLabel);

    QGroupBox *displayGroup = createSection(layout, "Display", this, "result.section.display");
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
    form->addRow("Field", m_fieldComboBox);

    m_scaleSpinBox = new QDoubleSpinBox(this);
    m_scaleSpinBox->setObjectName("result.deformationScale.spin");
    m_scaleSpinBox->setRange(0.0, 1000000.0);
    m_scaleSpinBox->setDecimals(3);
    m_scaleSpinBox->setSingleStep(1.0);
    m_scaleSpinBox->setSuffix(" x");
    form->addRow("Deformation", m_scaleSpinBox);

    m_meshEdgesCheckBox = new QCheckBox("Show mesh edges", this);
    m_meshEdgesCheckBox->setObjectName("result.meshEdges.checkbox");
    form->addRow("", m_meshEdgesCheckBox);

    m_undeformedOverlayCheckBox = new QCheckBox("Show undeformed overlay", this);
    m_undeformedOverlayCheckBox->setObjectName("result.undeformedOverlay.checkbox");
    form->addRow("", m_undeformedOverlayCheckBox);

    m_fieldUnitLabel = createStatusLabel("Current field: -, unit: -", this, "result.fieldUnit.label");
    form->addRow("Unit", m_fieldUnitLabel);

    m_lockScalarRangeCheckBox = new QCheckBox("Lock scalar range", this);
    m_lockScalarRangeCheckBox->setObjectName("result.scalarRange.lock");
    form->addRow("", m_lockScalarRangeCheckBox);

    m_scalarMinSpinBox = new QDoubleSpinBox(this);
    m_scalarMinSpinBox->setObjectName("result.scalarRange.min");
    configureRangeSpinBox(m_scalarMinSpinBox);
    form->addRow("Range min", m_scalarMinSpinBox);

    m_scalarMaxSpinBox = new QDoubleSpinBox(this);
    m_scalarMaxSpinBox->setObjectName("result.scalarRange.max");
    configureRangeSpinBox(m_scalarMaxSpinBox);
    form->addRow("Range max", m_scalarMaxSpinBox);

    QGroupBox *statusGroup = createSection(layout, "Status", this, "result.section.status");
    auto *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(5);
    m_scalarRangeLabel = createStatusLabel("Range: -", this, "result.scalarRange.label");
    m_coverageLabel = createStatusLabel("Coverage: -", this, "result.coverage.label");
    m_extremaLabel = createStatusLabel("Max: -", this, "result.extrema.label");
    m_fileStatusLabel = createStatusLabel("Files: -", this, "result.fileStatus.label");
    m_messagesLabel = createStatusLabel("-", this, "result.messages.label");
    m_probeLabel = createStatusLabel("Probe: -", this, "result.probe.label");
    statusLayout->addWidget(m_scalarRangeLabel);
    statusLayout->addWidget(m_coverageLabel);
    statusLayout->addWidget(m_extremaLabel);
    statusLayout->addWidget(m_fileStatusLabel);
    statusLayout->addWidget(m_messagesLabel);
    statusLayout->addWidget(m_probeLabel);

    QGroupBox *animationGroup = createSection(layout, "Animation", this, "result.section.animation");
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
    m_playAnimationButton = new QPushButton("Play", this);
    m_playAnimationButton->setObjectName("result.animation.play");
    m_stopAnimationButton = new QPushButton("Stop", this);
    m_stopAnimationButton->setObjectName("result.animation.stop");
    m_animationFrameLabel = createStatusLabel("Frame scale: -", this, "result.animationFrame.label");
    animationLayout->addWidget(new QLabel("Speed", this), 0, 0);
    animationLayout->addWidget(m_animationSpeedSpinBox, 0, 1, 1, 2);
    animationLayout->addWidget(m_playAnimationButton, 1, 1);
    animationLayout->addWidget(m_stopAnimationButton, 1, 2);
    animationLayout->addWidget(m_animationFrameLabel, 2, 0, 1, 3);

    QGroupBox *exportGroup = createSection(layout, "Export", this, "result.section.export");
    auto *exportLayout = new QGridLayout(exportGroup);
    exportLayout->setHorizontalSpacing(6);
    exportLayout->setVerticalSpacing(6);
    m_exportCsvButton = new QPushButton("Export CSV", this);
    m_exportCsvButton->setObjectName("result.export.csv");
    m_exportReportButton = new QPushButton("Export Report", this);
    m_exportReportButton->setObjectName("result.export.report");
    m_exportScreenshotButton = new QPushButton("Export Screenshot", this);
    m_exportScreenshotButton->setObjectName("result.export.screenshot");
    m_openDirectoryButton = new QPushButton("Open Directory", this);
    m_openDirectoryButton->setObjectName("result.export.openDirectory");
    exportLayout->addWidget(m_exportCsvButton, 0, 0);
    exportLayout->addWidget(m_exportReportButton, 0, 1);
    exportLayout->addWidget(m_exportScreenshotButton, 1, 0);
    exportLayout->addWidget(m_openDirectoryButton, 1, 1);

    QGroupBox *historyGroup = createSection(layout, "History", this, "result.section.history");
    auto *historyLayout = new QHBoxLayout(historyGroup);
    historyLayout->setSpacing(6);
    m_renameButton = new QPushButton("Rename", this);
    m_renameButton->setObjectName("result.history.rename");
    m_deleteButton = new QPushButton("Delete History", this);
    m_deleteButton->setObjectName("result.history.delete");
    historyLayout->addWidget(m_renameButton);
    historyLayout->addWidget(m_deleteButton);
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
        m_resultNameLabel->setText("No result selected");
        m_scalarRangeLabel->setText("Range: -");
        m_coverageLabel->setText("Coverage: -");
        m_extremaLabel->setText("Max: -");
        m_fileStatusLabel->setText("Files: -");
        m_messagesLabel->setText("-");
        m_probeLabel->setText("Probe: -");
        m_fieldUnitLabel->setText("Current field: -, unit: -");
        m_animationFrameLabel->setText("Frame scale: -");
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
    m_animationFrameLabel->setText(QString("Frame scale: %1 x").arg(resultObject->deformationScale, 0, 'g', 6));
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

    const QString rangeMode = resultObject->scalarRangeLocked ? "locked" : "auto";
    const QString fieldName = resultObject->displayFieldName.isEmpty()
        ? resultObject->primaryFieldName
        : resultObject->displayFieldName;
    const QString unit = ResultFieldMetadata::unitForField(fieldName);
    const double rangeMinimum = resultObject->scalarRangeLocked ? resultObject->lockedScalarMin : resultObject->scalarMin;
    const double rangeMaximum = resultObject->scalarRangeLocked ? resultObject->lockedScalarMax : resultObject->scalarMax;
    m_scalarRangeLabel->setText(QString("Range (%1): %2 to %3 %4")
        .arg(rangeMode)
        .arg(rangeMinimum, 0, 'g', 6)
        .arg(rangeMaximum, 0, 'g', 6)
        .arg(unit));
    m_fieldUnitLabel->setText(fieldUnitText(*resultObject));
    m_coverageLabel->setText("Coverage: " + coverageText(*resultObject));
    m_extremaLabel->setText(extremaText(*resultObject));
    m_fileStatusLabel->setText("Files: " + fileStatusText(*resultObject));
    m_messagesLabel->setText(resultObject->checkMessages.isEmpty()
        ? QString("Checks: OK")
        : "Checks: " + resultObject->checkMessages.join("; "));
    ResultProbe emptyProbe;
    m_probeLabel->setText(probeText(emptyProbe));
}

void ResultPostprocessPanel::setProbe(const ResultProbe &probe)
{
    m_probeLabel->setText(probeText(probe));
}
