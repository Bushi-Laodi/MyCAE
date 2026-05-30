#include "result/ResultFieldMetadata.h"

#include "solver/calculix/CalculiXResultGridBuilder.h"

QString ResultFieldMetadata::unitForField(const QString &fieldName)
{
    if (fieldName == CalculiXResultFields::VonMisesStress) {
        return "MPa";
    }
    if (fieldName.startsWith("RF", Qt::CaseInsensitive)
            || fieldName.contains("Reaction Force", Qt::CaseInsensitive)) {
        return "N";
    }
    return "mm";
}
