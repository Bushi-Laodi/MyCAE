#pragma once

#include <QString>
#include <QWidget>

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
    void setEnabledForResult(bool enabled);

signals:
    void fieldChanged(const QString &fieldName);
    void deformationScaleChanged(double scale);
    void meshEdgesChanged(bool enabled);
    void undeformedOverlayChanged(bool enabled);
    void exportScreenshotRequested();
    void openResultDirectoryRequested();
    void renameResultRequested();
    void deleteResultRequested();

private:
    void setStatusText(const ResultObject *resultObject);

    QComboBox *m_fieldComboBox = nullptr;
    QDoubleSpinBox *m_scaleSpinBox = nullptr;
    QCheckBox *m_meshEdgesCheckBox = nullptr;
    QCheckBox *m_undeformedOverlayCheckBox = nullptr;
    QLabel *m_resultNameLabel = nullptr;
    QLabel *m_scalarRangeLabel = nullptr;
    QLabel *m_coverageLabel = nullptr;
    QLabel *m_fileStatusLabel = nullptr;
    QLabel *m_messagesLabel = nullptr;
    QPushButton *m_exportScreenshotButton = nullptr;
    QPushButton *m_openDirectoryButton = nullptr;
    QPushButton *m_renameButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    bool m_updating = false;
};
