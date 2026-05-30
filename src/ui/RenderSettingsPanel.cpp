#include "ui/RenderSettingsPanel.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

RenderSettingsPanel::RenderSettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setSettings(RenderDisplaySettings{});
}

void RenderSettingsPanel::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    m_primaryOpacity = addOpacityControl(zh(u8"主模型不透明度"), 100);
    layout->addLayout(m_primaryOpacity.layout);
    m_resultOpacity = addOpacityControl(zh(u8"结果云图不透明度"), 96);
    layout->addLayout(m_resultOpacity.layout);
    m_undeformedOverlayOpacity = addOpacityControl(zh(u8"未变形轮廓不透明度"), 18);
    layout->addLayout(m_undeformedOverlayOpacity.layout);
    m_highlightOpacity = addOpacityControl(zh(u8"高亮不透明度"), 90);
    layout->addLayout(m_highlightOpacity.layout);

    m_geometryEdgesCheckBox = new QCheckBox(zh(u8"几何边线"), this);
    m_meshEdgesCheckBox = new QCheckBox(zh(u8"网格边线"), this);
    m_orientationMarkerCheckBox = new QCheckBox(zh(u8"坐标轴"), this);
    layout->addWidget(m_geometryEdgesCheckBox);
    layout->addWidget(m_meshEdgesCheckBox);
    layout->addWidget(m_orientationMarkerCheckBox);

    m_resetButton = new QPushButton(zh(u8"恢复默认"), this);
    layout->addWidget(m_resetButton);
    layout->addStretch(1);

    const auto connectSlider = [this](const OpacityControl &control) {
        connect(control.slider, &QSlider::valueChanged, this, [this, control]() {
            updateOpacityLabel(control);
            emitSettingsChanged();
        });
    };
    connectSlider(m_primaryOpacity);
    connectSlider(m_resultOpacity);
    connectSlider(m_undeformedOverlayOpacity);
    connectSlider(m_highlightOpacity);

    connect(m_geometryEdgesCheckBox, &QCheckBox::toggled, this, [this]() {
        emitSettingsChanged();
    });
    connect(m_meshEdgesCheckBox, &QCheckBox::toggled, this, [this]() {
        emitSettingsChanged();
    });
    connect(m_orientationMarkerCheckBox, &QCheckBox::toggled, this, [this]() {
        emitSettingsChanged();
    });
    connect(m_resetButton, &QPushButton::clicked, this, &RenderSettingsPanel::resetRequested);
}

RenderSettingsPanel::OpacityControl RenderSettingsPanel::addOpacityControl(
    const QString &labelText,
    int defaultValue
)
{
    OpacityControl control;
    control.layout = new QHBoxLayout;
    control.layout->setSpacing(8);

    auto *label = new QLabel(labelText, this);
    label->setMinimumWidth(118);
    control.layout->addWidget(label);

    control.slider = new QSlider(Qt::Horizontal, this);
    control.slider->setRange(10, 100);
    control.slider->setValue(defaultValue);
    control.layout->addWidget(control.slider, 1);

    control.valueLabel = new QLabel(this);
    control.valueLabel->setMinimumWidth(42);
    control.valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    control.layout->addWidget(control.valueLabel);

    updateOpacityLabel(control);
    return control;
}

void RenderSettingsPanel::setSettings(const RenderDisplaySettings &settings)
{
    const QSignalBlocker primaryBlocker(m_primaryOpacity.slider);
    const QSignalBlocker resultBlocker(m_resultOpacity.slider);
    const QSignalBlocker overlayBlocker(m_undeformedOverlayOpacity.slider);
    const QSignalBlocker highlightBlocker(m_highlightOpacity.slider);
    const QSignalBlocker geometryEdgesBlocker(m_geometryEdgesCheckBox);
    const QSignalBlocker meshEdgesBlocker(m_meshEdgesCheckBox);
    const QSignalBlocker orientationBlocker(m_orientationMarkerCheckBox);

    m_primaryOpacity.slider->setValue(opacityToPercent(settings.primaryOpacity));
    m_resultOpacity.slider->setValue(opacityToPercent(settings.resultOpacity));
    m_undeformedOverlayOpacity.slider->setValue(opacityToPercent(settings.undeformedOverlayOpacity));
    m_highlightOpacity.slider->setValue(opacityToPercent(settings.highlightOpacity));
    m_geometryEdgesCheckBox->setChecked(settings.geometryEdgesVisible);
    m_meshEdgesCheckBox->setChecked(settings.meshEdgesVisible);
    m_orientationMarkerCheckBox->setChecked(settings.orientationMarkerVisible);

    updateOpacityLabel(m_primaryOpacity);
    updateOpacityLabel(m_resultOpacity);
    updateOpacityLabel(m_undeformedOverlayOpacity);
    updateOpacityLabel(m_highlightOpacity);
}

RenderDisplaySettings RenderSettingsPanel::settings() const
{
    RenderDisplaySettings result;
    result.primaryOpacity = percentToOpacity(m_primaryOpacity.slider->value());
    result.resultOpacity = percentToOpacity(m_resultOpacity.slider->value());
    result.undeformedOverlayOpacity = percentToOpacity(m_undeformedOverlayOpacity.slider->value());
    result.highlightOpacity = percentToOpacity(m_highlightOpacity.slider->value());
    result.geometryEdgesVisible = m_geometryEdgesCheckBox->isChecked();
    result.meshEdgesVisible = m_meshEdgesCheckBox->isChecked();
    result.orientationMarkerVisible = m_orientationMarkerCheckBox->isChecked();
    return result;
}

void RenderSettingsPanel::updateOpacityLabel(const OpacityControl &control) const
{
    if (!control.slider || !control.valueLabel) {
        return;
    }
    control.valueLabel->setText(QString::number(control.slider->value()) + "%");
}

void RenderSettingsPanel::emitSettingsChanged()
{
    emit settingsChanged(settings());
}

int RenderSettingsPanel::opacityToPercent(double opacity)
{
    if (std::isnan(opacity)) {
        return 100;
    }
    return std::clamp(static_cast<int>(std::round(opacity * 100.0)), 10, 100);
}

double RenderSettingsPanel::percentToOpacity(int percent)
{
    return std::clamp(percent, 10, 100) / 100.0;
}
