#pragma once

#include "geometry/SphereGeometry.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class SphereDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit SphereDialog(QWidget *parent = nullptr);

    SphereGeometry sphereParameters() const;

private:
    QDoubleSpinBox *m_centerXSpinBox = nullptr;
    QDoubleSpinBox *m_centerYSpinBox = nullptr;
    QDoubleSpinBox *m_centerZSpinBox = nullptr;
    QDoubleSpinBox *m_radiusSpinBox = nullptr;
    QComboBox *m_unitComboBox = nullptr;
};
