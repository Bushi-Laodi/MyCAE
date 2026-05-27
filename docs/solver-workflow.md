# MyCAE 求解流程说明

本文档说明从工程数据到 CalculiX 结果显示的完整链路。

## 1. 总体数据流

```text
Project
  -> GeometryObject / FaceGroup
  -> MeshObject / MeshBoundary
  -> SimulationCase
  -> SolverPlugin
  -> Solver case directory
  -> ResultObject
  -> Result postprocess UI / VTK render
```

更具体地说：

```text
几何建模
-> 面组定义
-> Gmsh 网格生成
-> 材料/边界条件/载荷
-> CalculiX 输入 deck
-> ccx 求解
-> .dat/.sta/.frd/.log
-> 结果读取
-> VTK 网格着色和后处理面板
```

## 2. 工程数据

工程入口：

```text
project.json
```

主要运行时模型：

```text
ProjectModel
  GeometryRepository
  MeshRepository
  SolverRepository
  ResultRepository
  SelectionState
```

`ProjectModel` 是 UI 和 workflow 的主要数据入口，但具体持久化由各 manager/service 负责。

## 3. 几何阶段

关键文件：

```text
src/geometry/GeometryManager.*
src/geometry/GeometryCreationController.*
src/geometry/GeometryDisplayController.*
src/occ/OCCGeometryFactory.*
src/occ/OCCShapeIO.*
```

输出：

```text
geometry/*.json
geometry/*.brep
geometry/*.step
```

`.json` 保存 MyCAE 的几何元数据；`.brep/.step` 用于 OCC 加载和 Gmsh 网格。

## 4. 面组阶段

关键文件：

```text
src/geometry/FaceGroup.*
src/geometry/FaceGroupService.*
src/geometry/FaceGroupReferenceService.*
src/picking/PickController.*
```

面组的作用：

- 把用户拾取的几何面保存为可命名对象
- 作为边界条件和载荷的目标
- 在 Gmsh 导出时生成 Physical Surface
- 在 CalculiX 导出时映射为 surface / node set / element set

## 5. 网格阶段

关键文件：

```text
src/mesh/GmshCaseWriter.*
src/mesh/GmshRunner.*
src/mesh/MshReader.*
src/mesh/MeshBoundaryBuilder.*
src/mesh/MeshToVtkConverter.*
```

流程：

```text
STEP/BREP 几何
-> .geo
-> gmsh.exe
-> .msh
-> MeshData
-> MeshBoundary
-> VTK UnstructuredGrid
```

`MeshBoundaryBuilder` 从 `.msh` 中恢复表面物理组，使边界条件可以稳定绑定到网格边界。

## 6. 工况阶段

关键文件：

```text
src/solver/SimulationCase.*
src/solver/SimulationCaseBuilder.*
src/solver/SimulationCaseManager.*
src/solver/SolverDataService.*
```

持久化文件：

```text
solver/case.json
```

包含：

- 材料
- 边界条件
- 载荷
- 面组
- 网格设置

## 7. 求解器插件阶段

关键文件：

```text
src/solver/plugin/SolverPlugin.h
src/solver/plugin/SolverPluginManager.*
src/solver/plugin/BuiltInSolverPluginRegistry.*
src/solver/calculix/CalculiXPlugin.*
src/solver/openfoam/OpenFoamPlugin.*
```

插件能力用 capabilities 表达：

```text
export
run
read-result
```

当前状态：

```text
CalculiX：ready，支持 export/run/read-result
OpenFOAM：reserved，暂不支持实际能力
External Demo Solver：用于插件机制演示
```

## 8. CalculiX 导出和运行

关键文件：

```text
src/solver/calculix/CalculiXCaseDataBuilder.*
src/solver/calculix/CalculiXBoundaryMapper.*
src/solver/calculix/CalculiXInputDeckBuilder.*
src/solver/calculix/CalculiXDeckSectionWriter.*
src/solver/calculix/CalculiXRunner.*
src/solver/calculix/CalculiXResultReader.*
```

流程：

```text
ProjectModel
-> SimulationCase
-> CalculiXCaseData
-> .inp
-> ccx -i jobName
-> .sta/.dat/.frd/.log
-> SolverResultReadResult
-> ResultObject
```

输入 deck 主要包含：

- 节点
- 单元
- 材料
- NSET / ELSET / SURFACE
- 边界条件
- 载荷
- 静力分析步
- 输出请求

## 9. 结果读取和后处理

关键文件：

```text
src/result/ResultDataLoader.*
src/result/ResultDisplayController.*
src/result/ResultDisplaySummary.*
src/result/ResultExtremaCalculator.*
src/solver/calculix/CalculiXDatResultReader.*
src/solver/calculix/CalculiXResultGridBuilder.*
src/ui/ResultPostprocessPanel.*
src/ui/VtkRenderCanvas.*
```

数据流：

```text
ResultObject
-> ResultDataLoader
-> MeshData + CalculiXDatResult
-> ResultDisplaySummary 或 CalculiXResultGridBuilder
-> ResultObject 更新
-> ResultPostprocessPanel
-> RenderView / VtkRenderCanvas
```

字段：

```text
Ux
Uy
Uz
Displacement Magnitude
Von Mises Stress
```

## 10. 自动化验收中的特殊路径

UI 验收和截图采集会启用稳定摘要模式：

```text
mycae.skipResultRender = true
```

作用：

- 打开真实样例工程
- 选择真实结果节点
- 填充结果后处理面板
- 不重建结果 VTK 网格

这样可以避免短生命周期自动化窗口在 Debug VTK/Qt 环境下触发释放期崩溃。普通用户交互路径不受影响，仍会走完整 VTK 结果显示。
