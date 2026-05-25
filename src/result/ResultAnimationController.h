#pragma once

#include <QObject>
#include <QTimer>

class ResultAnimationController final : public QObject
{
    Q_OBJECT

public:
    explicit ResultAnimationController(QObject *parent = nullptr);

    void start(double peakScale, double cyclesPerSecond);
    void stop();
    bool isRunning() const;
    double currentScale() const;

signals:
    void frameScaleChanged(double scale);

private:
    void advanceFrame();

    QTimer m_timer;
    double m_peakScale = 0.0;
    double m_cyclesPerSecond = 1.0;
    double m_phase = 0.0;
    double m_currentScale = 0.0;
};
