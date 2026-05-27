#include "ui/ResultPostprocessText.h"

#include "result/ResultExtrema.h"
#include "result/ResultFieldMetadata.h"
#include "result/ResultObject.h"

namespace
{
QString zh(const char *text)
{
    return QString::fromUtf8(text);
}
}

namespace ResultPostprocessText
{
QString coverage(const ResultObject &resultObject)
{
    const QString nodeText = resultObject.meshNodeCount > 0
        ? zh(u8"节点 %1/%2").arg(resultObject.matchedNodeCount).arg(resultObject.meshNodeCount)
        : zh(u8"节点 -");
    const QString elementText = resultObject.meshElementCount > 0
        ? zh(u8"单元 %1/%2").arg(resultObject.matchedElementCount).arg(resultObject.meshElementCount)
        : zh(u8"单元 -");
    return nodeText + zh(u8"，") + elementText;
}

QString fileStatus(const ResultObject &resultObject)
{
    return resultObject.resultFilesComplete ? zh(u8"结果文件完整") : zh(u8"结果文件不完整");
}

QString nodeExtreme(const ResultNodeExtreme &extreme)
{
    if (!extreme.valid) {
        return "-";
    }
    return zh(u8"%1：节点 %2，值 %3，坐标 (%4, %5, %6)")
        .arg(extreme.fieldName)
        .arg(extreme.nodeId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString elementExtreme(const ResultElementExtreme &extreme)
{
    if (!extreme.valid) {
        return "-";
    }
    return zh(u8"%1：单元 %2，值 %3，中心 (%4, %5, %6)")
        .arg(extreme.fieldName)
        .arg(extreme.elementId)
        .arg(extreme.value, 0, 'g', 6)
        .arg(extreme.x, 0, 'g', 6)
        .arg(extreme.y, 0, 'g', 6)
        .arg(extreme.z, 0, 'g', 6);
}

QString marker(const ResultExtremeMarker &marker)
{
    if (!marker.valid) {
        return "-";
    }
    return zh(u8"%1 %2，值 %3")
        .arg(marker.element ? zh(u8"单元") : zh(u8"节点"))
        .arg(marker.id)
        .arg(marker.value, 0, 'g', 6);
}

QString fieldUnit(const ResultObject &resultObject)
{
    const QString fieldName = resultObject.displayFieldName.isEmpty()
        ? resultObject.primaryFieldName
        : resultObject.displayFieldName;
    const QString unit = ResultFieldMetadata::unitForField(fieldName);
    return zh(u8"当前场：") + fieldName + zh(u8"，单位：") + unit;
}

QString coordinate(double x, double y, double z)
{
    return QString("(%1, %2, %3)").arg(x, 0, 'g', 6).arg(y, 0, 'g', 6).arg(z, 0, 'g', 6);
}
}
