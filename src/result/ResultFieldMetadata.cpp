#include "result/ResultFieldMetadata.h"

#include "solver/calculix/CalculiXResultGridBuilder.h"

QString ResultFieldMetadata::unitForField(const QString &fieldName)
{
    return fieldName == CalculiXResultFields::VonMisesStress ? "Pa" : "model length";
}
