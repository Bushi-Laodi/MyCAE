#pragma once

#include "geometry/BoxGeometry.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class BoxDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit BoxDialog(QWidget *parent = nullptr);

    BoxGeometry boxParameters() const;

private:
    QDoubleSpinBox *m_lengthSpinBox = nullptr;
    QDoubleSpinBox *m_widthSpinBox = nullptr;
    QDoubleSpinBox *m_heightSpinBox = nullptr;
    QComboBox *m_unitComboBox = nullptr;
};
