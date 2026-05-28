#pragma once

#include "geometry/GeometryTransform.h"

#include <QDialog>

class QCheckBox;
class QDoubleSpinBox;

class GeometryTransformDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit GeometryTransformDialog(const GeometryCenter &currentCenter, QWidget *parent = nullptr);

    GeometryTransformParameters parameters() const;

private:
    void setupUi(const GeometryCenter &currentCenter);
    void updateAbsoluteCenterState();

    QCheckBox *m_absoluteCenterCheck = nullptr;
    QDoubleSpinBox *m_targetCenterX = nullptr;
    QDoubleSpinBox *m_targetCenterY = nullptr;
    QDoubleSpinBox *m_targetCenterZ = nullptr;
    QDoubleSpinBox *m_translateX = nullptr;
    QDoubleSpinBox *m_translateY = nullptr;
    QDoubleSpinBox *m_translateZ = nullptr;
    QDoubleSpinBox *m_rotateX = nullptr;
    QDoubleSpinBox *m_rotateY = nullptr;
    QDoubleSpinBox *m_rotateZ = nullptr;
    QDoubleSpinBox *m_uniformScale = nullptr;
};
