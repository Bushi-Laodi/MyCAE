#include "OccEnvironmentCheck.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

OccEnvironmentCheckResult runOccEnvironmentCheck()
{
    try {
        BRepPrimAPI_MakeBox boxMaker(10.0, 20.0, 30.0);
        const TopoDS_Shape shape = boxMaker.Shape();

        if (shape.IsNull()) {
            return {false, "Open CASCADE 测试失败：BRepPrimAPI_MakeBox 生成了空 Shape。"};
        }

        return {true, "Open CASCADE 测试通过：已成功创建 10 x 20 x 30 mm 的 TopoDS_Shape。"};
    } catch (const Standard_Failure &failure) {
        return {false, QString("Open CASCADE 测试异常：%1").arg(failure.what())};
    } catch (...) {
        return {false, "Open CASCADE 测试异常：未知错误。"};
    }
}
