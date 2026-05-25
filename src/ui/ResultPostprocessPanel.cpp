#include "ui/ResultPostprocessPanel.h"

#include "result/ResultObject.h"
#include "solver/calculix/CalculiXResultGridBuilder.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
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

QString nodeExtremeText(const ResultNodeExtreme &extreme)
{
    if (!extreme.valid) {
        return "Max: -";
    }
    return QString("Max %1: node %2, value %3, (%4, %5, %6)")
        .arg(extreme.fieldName)
        .arg(extreme.nodeId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString elementExtremeText(const ResultElementExtreme &extreme)
{
    if (!extreme.valid) {
        return "Max: -";
    }
    return QString("Max %1: element %2, value %3, centroid (%4, %5, %6)")
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
    lines.append(nodeExtremeText(resultObject.extrema.maxDisplacementMagnitude));
    lines.append(nodeExtremeText(resultObject.extrema.maxUx));
    lines.append(nodeExtremeText(resultObject.extrema.maxUy));
    lines.append(nodeExtremeText(resultObject.extrema.maxUz));
    lines.append(elementExtremeText(resultObject.extrema.maxVonMisesStress));
    if (resultObject.extrema.selectedMarker.valid) {
        const QString idLabel = resultObject.extrema.selectedMarker.element ? "element" : "node";
        lines.prepend(QString("Current max: %1 %2, value %3")
            .arg(idLabel)
            .arg(resultObject.extrema.selectedMarker.id)
            .arg(resultObject.extrema.selectedMarker.value, 0, 'g', 6));
    }
    return lines.join("\n");
}
}

ResultPostprocessPanel::ResultPostprocessPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    m_resultNameLabel = new QLabel("No result selected", this);
    m_resultNameLabel->setWordWrap(true);
    layout->addWidget(m_resultNameLabel);

    auto *form = new QFormLayout;
    m_fieldComboBox = new QComboBox(this);
    m_fieldComboBox->addItems(QStringList{
        CalculiXResultFields::Ux,
        CalculiXResultFields::Uy,
        CalculiXResultFields::Uz,
        CalculiXResultFields::DisplacementMagnitude,
        CalculiXResultFields::VonMisesStress
    });
    form->addRow("Field", m_fieldComboBox);

    m_scaleSpinBox = new QDoubleSpinBox(this);
    m_scaleSpinBox->setRange(0.0, 1000000.0);
    m_scaleSpinBox->setDecimals(3);
    m_scaleSpinBox->setSingleStep(1.0);
    m_scaleSpinBox->setSuffix(" x");
    form->addRow("Deformation", m_scaleSpinBox);

    m_meshEdgesCheckBox = new QCheckBox("Show mesh edges", this);
    form->addRow("", m_meshEdgesCheckBox);

    m_undeformedOverlayCheckBox = new QCheckBox("Show undeformed overlay", this);
    form->addRow("", m_undeformedOverlayCheckBox);

    m_animationSpeedSpinBox = new QDoubleSpinBox(this);
    m_animationSpeedSpinBox->setRange(0.1, 5.0);
    m_animationSpeedSpinBox->setDecimals(2);
    m_animationSpeedSpinBox->setSingleStep(0.25);
    m_animationSpeedSpinBox->setValue(1.0);
    m_animationSpeedSpinBox->setSuffix(" Hz");
    form->addRow("Animation speed", m_animationSpeedSpinBox);
    layout->addLayout(form);

    m_scalarRangeLabel = new QLabel("Range: -", this);
    m_coverageLabel = new QLabel("Coverage: -", this);
    m_extremaLabel = new QLabel("Max: -", this);
    m_extremaLabel->setWordWrap(true);
    m_fileStatusLabel = new QLabel("Files: -", this);
    m_messagesLabel = new QLabel("-", this);
    m_messagesLabel->setWordWrap(true);
    layout->addWidget(m_scalarRangeLabel);
    layout->addWidget(m_coverageLabel);
    layout->addWidget(m_extremaLabel);
    layout->addWidget(m_fileStatusLabel);
    layout->addWidget(m_messagesLabel);

    auto *firstButtonRow = new QHBoxLayout;
    m_playAnimationButton = new QPushButton("Play", this);
    m_stopAnimationButton = new QPushButton("Stop", this);
    firstButtonRow->addWidget(m_playAnimationButton);
    firstButtonRow->addWidget(m_stopAnimationButton);
    layout->addLayout(firstButtonRow);

    m_animationFrameLabel = new QLabel("Frame scale: -", this);
    layout->addWidget(m_animationFrameLabel);

    auto *secondButtonRow = new QHBoxLayout;
    m_exportCsvButton = new QPushButton("Export CSV", this);
    m_exportReportButton = new QPushButton("Export Report", this);
    m_exportScreenshotButton = new QPushButton("Export Screenshot", this);
    m_openDirectoryButton = new QPushButton("Open Directory", this);
    secondButtonRow->addWidget(m_exportCsvButton);
    secondButtonRow->addWidget(m_exportReportButton);
    secondButtonRow->addWidget(m_exportScreenshotButton);
    secondButtonRow->addWidget(m_openDirectoryButton);
    layout->addLayout(secondButtonRow);

    auto *thirdButtonRow = new QHBoxLayout;
    m_renameButton = new QPushButton("Rename", this);
    m_deleteButton = new QPushButton("Delete History", this);
    thirdButtonRow->addWidget(m_renameButton);
    thirdButtonRow->addWidget(m_deleteButton);
    layout->addLayout(thirdButtonRow);
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

    m_scalarRangeLabel->setText(QString("Range: %1 to %2")
        .arg(resultObject->scalarMin, 0, 'g', 6)
        .arg(resultObject->scalarMax, 0, 'g', 6));
    m_coverageLabel->setText("Coverage: " + coverageText(*resultObject));
    m_extremaLabel->setText(extremaText(*resultObject));
    m_fileStatusLabel->setText("Files: " + fileStatusText(*resultObject));
    m_messagesLabel->setText(resultObject->checkMessages.isEmpty()
        ? QString("Checks: OK")
        : "Checks: " + resultObject->checkMessages.join("; "));
}
