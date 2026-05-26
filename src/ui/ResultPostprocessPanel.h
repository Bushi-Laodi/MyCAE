#pragma once

#include <QString>
#include <QWidget>

struct ResultProbe;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
struct ResultObject;

class ResultPostprocessPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit ResultPostprocessPanel(QWidget *parent = nullptr);

    void setResult(const ResultObject *resultObject);
    void setProbe(const ResultProbe &probe);
    void setEnabledForResult(bool enabled);

signals:
    void fieldChanged(const QString &fieldName);
    void deformationScaleChanged(double scale);
    void meshEdgesChanged(bool enabled);
    void undeformedOverlayChanged(bool enabled);
    void scalarRangeLockChanged(bool locked);
    void scalarRangeChanged(double minimum, double maximum);
    void animationPlayRequested(double speed);
    void animationStopRequested();
    void exportCsvRequested();
    void exportReportRequested();
    void exportScreenshotRequested();
    void openResultDirectoryRequested();
    void renameResultRequested();
    void deleteResultRequested();

private:
    void setStatusText(const ResultObject *resultObject);

    QLabel *m_probeHintLabel = nullptr;
    QLabel *m_probeNodeIdValue = nullptr;
    QLabel *m_probeElementIdValue = nullptr;
    QLabel *m_probeCoordinateValue = nullptr;
    QLabel *m_probeUxValue = nullptr;
    QLabel *m_probeUyValue = nullptr;
    QLabel *m_probeUzValue = nullptr;
    QLabel *m_probeMagnitudeValue = nullptr;
    QLabel *m_probeVonMisesValue = nullptr;
    QLabel *m_currentMinValue = nullptr;
    QLabel *m_currentMaxValue = nullptr;
    QLabel *m_maxDisplacementValue = nullptr;
    QLabel *m_maxUxValue = nullptr;
    QLabel *m_maxUyValue = nullptr;
    QLabel *m_maxUzValue = nullptr;
    QLabel *m_maxVonMisesValue = nullptr;
    QComboBox *m_fieldComboBox = nullptr;
    QDoubleSpinBox *m_scaleSpinBox = nullptr;
    QCheckBox *m_meshEdgesCheckBox = nullptr;
    QCheckBox *m_undeformedOverlayCheckBox = nullptr;
    QCheckBox *m_lockScalarRangeCheckBox = nullptr;
    QDoubleSpinBox *m_scalarMinSpinBox = nullptr;
    QDoubleSpinBox *m_scalarMaxSpinBox = nullptr;
    QLabel *m_resultNameLabel = nullptr;
    QLabel *m_fieldUnitLabel = nullptr;
    QLabel *m_scalarRangeLabel = nullptr;
    QLabel *m_coverageLabel = nullptr;
    QLabel *m_fileStatusLabel = nullptr;
    QLabel *m_messagesLabel = nullptr;
    QDoubleSpinBox *m_animationSpeedSpinBox = nullptr;
    QLabel *m_animationFrameLabel = nullptr;
    QPushButton *m_playAnimationButton = nullptr;
    QPushButton *m_stopAnimationButton = nullptr;
    QPushButton *m_exportCsvButton = nullptr;
    QPushButton *m_exportReportButton = nullptr;
    QPushButton *m_exportScreenshotButton = nullptr;
    QPushButton *m_openDirectoryButton = nullptr;
    QPushButton *m_renameButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    bool m_updating = false;
};
