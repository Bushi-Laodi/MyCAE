#include "result/ResultAnimationController.h"

#include <algorithm>
#include <cmath>

ResultAnimationController::ResultAnimationController(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(33);
    connect(&m_timer, &QTimer::timeout, this, &ResultAnimationController::advanceFrame);
}

void ResultAnimationController::start(double peakScale, double cyclesPerSecond)
{
    m_peakScale = std::max(0.0, peakScale);
    m_cyclesPerSecond = std::clamp(cyclesPerSecond, 0.1, 5.0);
    m_phase = 0.0;
    m_currentScale = 0.0;
    emit frameScaleChanged(m_currentScale);
    m_timer.start();
}

void ResultAnimationController::stop()
{
    m_timer.stop();
}

bool ResultAnimationController::isRunning() const
{
    return m_timer.isActive();
}

double ResultAnimationController::currentScale() const
{
    return m_currentScale;
}

void ResultAnimationController::advanceFrame()
{
    m_phase += (static_cast<double>(m_timer.interval()) / 1000.0) * m_cyclesPerSecond;
    m_phase -= std::floor(m_phase);

    const double normalizedScale = 1.0 - std::abs(2.0 * m_phase - 1.0);
    m_currentScale = m_peakScale * normalizedScale;
    emit frameScaleChanged(m_currentScale);
}
