#pragma once

#include "geometry/CylinderGeometry.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class CylinderDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit CylinderDialog(QWidget *parent = nullptr);

    CylinderGeometry cylinderParameters() const;

private:
    QDoubleSpinBox *m_radiusSpinBox = nullptr;
    QDoubleSpinBox *m_heightSpinBox = nullptr;
    QComboBox *m_unitComboBox = nullptr;
};
