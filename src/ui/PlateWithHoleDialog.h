#pragma once

#include "geometry/PlateWithHoleGeometry.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class PlateWithHoleDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit PlateWithHoleDialog(QWidget *parent = nullptr);

    PlateWithHoleGeometry plateParameters() const;

private:
    bool validate();
    void updateSuffixes(const QString &unit);

    QDoubleSpinBox *m_lengthSpinBox = nullptr;
    QDoubleSpinBox *m_widthSpinBox = nullptr;
    QDoubleSpinBox *m_thicknessSpinBox = nullptr;
    QDoubleSpinBox *m_holeRadiusSpinBox = nullptr;
    QDoubleSpinBox *m_centerXSpinBox = nullptr;
    QDoubleSpinBox *m_centerYSpinBox = nullptr;
    QDoubleSpinBox *m_centerZSpinBox = nullptr;
    QComboBox *m_unitComboBox = nullptr;
};
