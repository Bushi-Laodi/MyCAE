#pragma once

#include "geometry/GeometryBooleanOperation.h"
#include "geometry/GeometryObject.h"

#include <QDialog>
#include <QVector>

#include <optional>

class QComboBox;
class QLineEdit;
class QWidget;

struct BooleanOperationDialogResult
{
    QString leftGeometryName;
    QString rightGeometryName;
    GeometryBooleanOperationType operationType = GeometryBooleanOperationType::Union;
    QString resultName;
};

class BooleanOperationDialog final : public QDialog
{
public:
    explicit BooleanOperationDialog(
        const QVector<GeometryObject> &geometries,
        const QString &preferredLeftGeometryName,
        QWidget *parent = nullptr
    );

    BooleanOperationDialogResult operation() const;

    static std::optional<BooleanOperationDialogResult> getOperation(
        QWidget *parent,
        const QVector<GeometryObject> &geometries,
        const QString &preferredLeftGeometryName
    );

private:
    void setupUi(const QVector<GeometryObject> &geometries, const QString &preferredLeftGeometryName);
    bool validate();
    void setComboText(QComboBox *combo, const QString &text);

    QComboBox *m_leftGeometryCombo = nullptr;
    QComboBox *m_rightGeometryCombo = nullptr;
    QComboBox *m_operationCombo = nullptr;
    QLineEdit *m_resultNameEdit = nullptr;
};
