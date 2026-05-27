#pragma once

#include <QString>

struct ResultElementExtreme;
struct ResultExtremeMarker;
struct ResultNodeExtreme;
struct ResultObject;

namespace ResultPostprocessText
{
QString coverage(const ResultObject &resultObject);
QString fileStatus(const ResultObject &resultObject);
QString nodeExtreme(const ResultNodeExtreme &extreme);
QString elementExtreme(const ResultElementExtreme &extreme);
QString marker(const ResultExtremeMarker &marker);
QString fieldUnit(const ResultObject &resultObject);
QString coordinate(double x, double y, double z);
}
