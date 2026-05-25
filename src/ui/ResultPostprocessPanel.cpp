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
    layout->addLayout(form);

    m_scalarRangeLabel = new QLabel("Range: -", this);
    m_coverageLabel = new QLabel("Coverage: -", this);
    m_fileStatusLabel = new QLabel("Files: -", this);
    m_messagesLabel = new QLabel("-", this);
    m_messagesLabel->setWordWrap(true);
    layout->addWidget(m_scalarRangeLabel);
    layout->addWidget(m_coverageLabel);
    layout->addWidget(m_fileStatusLabel);
    layout->addWidget(m_messagesLabel);

    auto *firstButtonRow = new QHBoxLayout;
    m_exportScreenshotButton = new QPushButton("Export Screenshot", this);
    m_openDirectoryButton = new QPushButton("Open Directory", this);
    firstButtonRow->addWidget(m_exportScreenshotButton);
    firstButtonRow->addWidget(m_openDirectoryButton);
    layout->addLayout(firstButtonRow);

    auto *secondButtonRow = new QHBoxLayout;
    m_renameButton = new QPushButton("Rename", this);
    m_deleteButton = new QPushButton("Delete History", this);
    secondButtonRow->addWidget(m_renameButton);
    secondButtonRow->addWidget(m_deleteButton);
    layout->addLayout(secondButtonRow);
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
        m_fileStatusLabel->setText("Files: -");
        m_messagesLabel->setText("-");
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
    m_fileStatusLabel->setText("Files: " + fileStatusText(*resultObject));
    m_messagesLabel->setText(resultObject->checkMessages.isEmpty()
        ? QString("Checks: OK")
        : "Checks: " + resultObject->checkMessages.join("; "));
}
