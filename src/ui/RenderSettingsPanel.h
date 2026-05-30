#pragma once

#include "render/RenderDisplaySettings.h"

#include <QString>
#include <QWidget>

class QCheckBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QSlider;

class RenderSettingsPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit RenderSettingsPanel(QWidget *parent = nullptr);

    void setSettings(const RenderDisplaySettings &settings);
    RenderDisplaySettings settings() const;

signals:
    void settingsChanged(const RenderDisplaySettings &settings);
    void resetRequested();

private:
    struct OpacityControl
    {
        QHBoxLayout *layout = nullptr;
        QSlider *slider = nullptr;
        QLabel *valueLabel = nullptr;
    };

    void setupUi();
    OpacityControl addOpacityControl(const QString &labelText, int defaultValue);
    void updateOpacityLabel(const OpacityControl &control) const;
    void emitSettingsChanged();
    static int opacityToPercent(double opacity);
    static double percentToOpacity(int percent);

    OpacityControl m_primaryOpacity;
    OpacityControl m_resultOpacity;
    OpacityControl m_undeformedOverlayOpacity;
    OpacityControl m_highlightOpacity;
    QCheckBox *m_geometryEdgesCheckBox = nullptr;
    QCheckBox *m_meshEdgesCheckBox = nullptr;
    QCheckBox *m_orientationMarkerCheckBox = nullptr;
    QPushButton *m_resetButton = nullptr;
};
