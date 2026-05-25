# MyCAE 完整功能测试教程

本文档用于测试当前阶段 MyCAE 的完整主流程：

```text
项目
  -> 几何
  -> 面拾取 / FaceGroup
  -> 网格
  -> 材料
  -> 边界条件
  -> 载荷
  -> SolverPlugin
  -> CalculiX 导出 / 运行 / 读取结果
  -> ResultObject 展示
```

当前阶段的重点不是完整 OpenFOAM 求解，而是验证：

```text
CalculiX 跑通结构仿真主线；
OpenFOAM 作为 reserved placeholder 能被插件机制表达；
外部 demo solver 和内置求解器能共存在统一 SolverPlugin 列表中。
```

## 1. 测试前准备

### 1.1 构建 MyCAE

使用你当前项目的 Qt / CMake / MSVC 环境构建 `MyCAE`。

本文档不限定具体构建命令，但构建成功后需要能启动：

```text
MyCAE.exe
```

启动后底部 `Log` 面板应出现类似信息：

```text
MyCAE started.
VTK renderer initialized.
Registered solver plugin: CalculiX (calculix), source=built-in, status=ready, capabilities=export, run, read-result.
Registered solver plugin: OpenFOAM (openfoam), source=built-in, status=reserved, capabilities=none.
Registered solver plugin: External Demo Solver (demo), source=external: ..., status=...
```

如果没有看到 `CalculiX` 插件注册信息，优先检查：

```text
src/solver/plugin/BuiltInSolverPluginRegistry.cpp
src/solver/plugin/SolverPluginManager.cpp
```

### 1.2 检查 Gmsh

菜单：

```text
Mesh -> Check Gmsh
```

期望：

```text
Gmsh environment check succeeded.
```

如果失败，检查 `CMakeLists.txt` 中：

```text
MYCAE_GMSH_EXECUTABLE
```

应指向真实存在的 `gmsh.exe`。

### 1.3 检查 CalculiX

CalculiX 默认通过以下优先级寻找 `ccx`：

```text
1. 环境变量 MYCAE_CALCULIX_EXECUTABLE
2. CMake 编译定义 MYCAE_CALCULIX_EXECUTABLE_PATH
3. PATH 中的 ccx
```

推荐设置环境变量：

```powershell
$env:MYCAE_CALCULIX_EXECUTABLE="D:\path\to\ccx.exe"
```

或者在 CMake 中配置：

```text
MYCAE_CALCULIX_EXECUTABLE = D:/path/to/ccx.exe
```

## 2. 创建测试项目

菜单：

```text
File -> New Project
```

选择一个空目录，例如：

```text
D:\MyCAE_Test\case_001
```

期望：

```text
Project / Model 面板出现项目根节点；
Log 面板显示 Project created；
项目目录下生成 project.json 和 simulation case 相关文件。
```

## 3. 创建几何

### 3.1 创建 Box

菜单：

```text
Geometry -> Create Box
```

建议参数：

```text
Length = 100 mm
Width  = 20 mm
Height = 20 mm
Unit   = mm
```

点击 `OK`。

期望：

```text
左侧 Geometry 下出现 Box 几何；
中间 RenderView 显示长方体；
右侧 Properties 显示几何名称、尺寸、STEP 路径；
项目目录下生成 geometry / STEP 相关文件。
```

### 3.2 可选：创建 Cylinder

菜单：

```text
Geometry -> Create Cylinder
```

建议参数：

```text
Radius = 20 mm
Height = 100 mm
Unit   = mm
```

该步骤用于验证多种 OCC 几何创建，不是 CalculiX 闭环必需项。

## 4. 测试拾取和 FaceGroup

### 4.1 选择几何

在左侧项目树中选择刚创建的 Box。

期望：

```text
右侧 Properties 显示 Box 信息；
RenderView 显示该几何；
Mesh -> Generate Mesh 变为可用。
```

### 4.2 开启面拾取

菜单：

```text
Picking -> Pick Face
```

在 RenderView 中点击 Box 的一个端面。

期望：

```text
被选中的面高亮；
Properties 显示当前 Pick 状态；
Log 显示 picked face 信息；
Picking -> Create Face Group from Pick 变为可用。
```

### 4.3 创建 FaceGroup

菜单：

```text
Picking -> Create Face Group from Pick
```

期望：

```text
Project / Model -> Face Groups 下出现新的 FaceGroup；
选择该 FaceGroup 后，Properties 显示 face indices / face references；
RenderView 能高亮该 FaceGroup。
```

### 4.4 设置物理组标记

选择刚创建的 FaceGroup。

菜单：

```text
Picking -> Toggle Face Group Physical Group
```

期望：

```text
FaceGroup 的 physicalGroupEnabled 状态被切换；
Log 显示 FaceGroup 更新；
```

注意：当前 `GmshCaseWriter` 已经能扫描 FaceGroup 请求，但 OCC face index 到 Gmsh surface tag 的稳定映射仍是保守实现。如果 Log 中出现：

```text
Gmsh physical groups and local mesh rules were not written
```

这是当前阶段的预期限制，不代表基础网格失败。

## 5. 生成网格

### 5.1 生成 tetra4 网格

选择 Box 几何。

菜单：

```text
Mesh -> Generate Mesh
```

期望：

```text
Log 显示 gmsh 命令；
项目目录下生成 mesh/<geometry>.msh；
项目目录下生成 mesh/<geometry>_mesh.json；
Project / Model -> Mesh 下出现 <geometry>_Mesh；
Properties 显示 nodeCount / tetraCount；
```

### 5.2 读取网格信息

选择 Box 几何。

菜单：

```text
Mesh -> Read Mesh Info
```

期望：

```text
Log 显示 Node count 和 Tetra count；
数量应大于 0。
```

### 5.3 显示网格

选择 Box 几何或 Mesh 节点。

菜单：

```text
Mesh -> Show Mesh
```

期望：

```text
RenderView 显示 tetra 网格；
Log 显示 Mesh displayed；
```

## 6. 创建求解数据

### 6.1 创建 Solid 材料

菜单：

```text
Solver Setup -> Create Material
```

建议参数：

```text
Name           = Steel
Domain         = Solid
Enable Density = true
Density        = 7800 kg/m^3
Young Modulus  = 2.1e11 Pa
Poisson Ratio  = 0.3
```

期望：

```text
Project / Model -> Materials 下出现 Steel；
Properties 显示 youngModulus / poissonRatio；
simulation case 被保存。
```

### 6.2 创建固定端边界条件

菜单：

```text
Solver Setup -> Create Boundary Condition
```

建议参数：

```text
Name        = FixedEnd
Type        = Wall
Geometry    = 选择 Box 几何
Face Group  = 选择固定端 FaceGroup
Material ID = steel
```

说明：

```text
当前 CalculiX 映射中，Wall 且没有载荷引用时，会作为固定约束导出。
```

期望：

```text
Project / Model -> Boundary Conditions 下出现 FixedEnd；
Properties 显示 target geometry / face group / material id；
```

### 6.3 创建受力端边界条件

再拾取 Box 的另一个端面并创建 FaceGroup，例如：

```text
LoadEnd
```

然后创建边界条件：

```text
Name        = LoadEndBC
Type        = Wall
Geometry    = Box
Face Group  = LoadEnd
Material ID = steel
```

### 6.4 创建载荷

菜单：

```text
Solver Setup -> Create Load
```

建议参数：

```text
Name                  = EndPressure
Type                  = Pressure
Boundary Condition ID = loadendbc
Field Name            = pressure
Value                 = 100000
Unit                  = Pa
```

注意：

```text
Boundary Condition ID 必须填写边界条件 id。
当前 id 规则通常是名称小写并将空格替换为下划线。
例如 LoadEndBC -> loadendbc。
```

期望：

```text
Project / Model -> Loads 下出现 EndPressure；
Properties 显示 boundaryConditionId、fieldName、value；
```

## 7. 运行 SolverPlugin

### 7.1 查看 Simulation 菜单

菜单：

```text
Simulation
```

应至少看到：

```text
Run CalculiX
Run OpenFOAM (reserved)
Run External Demo Solver
```

期望状态：

```text
Run CalculiX 可点击；
Run OpenFOAM (reserved) 不可点击或点击后明确提示 reserved / unavailable；
External Demo Solver 取决于外部插件配置是否可用。
```

### 7.2 运行 CalculiX

菜单：

```text
Simulation -> Run CalculiX
```

内部流程应为：

```text
保存 simulation case
构造 SolverCaseContext
CalculiXCaseWriter 导出 .inp
CalculiXRunner 调用 ccx
CalculiXResultReader 读取 .sta / .dat / .frd / .log
生成 ResultObject
```

期望 Log：

```text
Solver plugin: CalculiX (calculix)
Solver family: structural
Solver case directory: <project>/solver/calculix/<case>_<timestamp>
CalculiX case data: <N> nodes, <M> tetrahedra, ...
CalculiX input written: ... .inp
CalculiX command: ccx <jobName>
CalculiX log: ... .log
CalculiX result directory: ...
Solver result: CalculiX result read: ...
```

期望文件：

```text
<project>/solver/calculix/<run>/
  <jobName>.inp
  <jobName>.log
  <jobName>.sta
  <jobName>.dat
  <jobName>.frd
```

期望 UI：

```text
Project / Model -> Results 下出现 CalculiX Result；
点击结果后，Properties 显示：
  ID
  Solver
  Case Path
  Log File
  Created At
  Success
  Summary
```

## 8. OpenFOAM placeholder 验证

菜单：

```text
Simulation -> Run OpenFOAM (reserved)
```

当前阶段 OpenFOAM 不应完整运行。

期望：

```text
插件列表能显示 OpenFOAM；
状态为 reserved；
不会影响 CalculiX；
如果触发到运行边界，应返回明确 not implemented / reserved 信息。
```

这一步用于证明：

```text
SolverPlugin 机制能表达不同求解器；
OpenFOAM 接口已预留；
第一阶段没有把 CFD case writer 作为主目标。
```

## 9. 外部 demo solver 验证

如果 `resource/solver/demo_solver` 被复制到运行目录，菜单中应出现：

```text
Run External Demo Solver
```

运行后期望：

```text
生成 solver/demo/<timestamp> 或对应 case 目录；
写入 case.json；
执行 demo_solver.py；
读取 result.json；
Results 下出现 External Demo Solver Result；
```

如果该项不可用，检查：

```text
resource/solver/demo_solver/solver_plugin.json
resource/solver/demo_solver/demo_solver.py
构建后 exe 目录下的 resource/solver/demo_solver
```

## 10. 验收清单

完整功能测试通过时，应满足：

```text
1. 能启动 MyCAE，VTK 渲染初始化成功
2. 能新建项目
3. 能创建 Box / Cylinder 几何
4. 能选择几何并显示属性
5. 能拾取面
6. 能创建 / 选择 / 高亮 FaceGroup
7. 能检查 Gmsh
8. 能生成 tetra4 MSH 网格
9. 能读取并显示网格
10. 能创建 Solid 材料
11. 能创建边界条件
12. 能创建载荷
13. Simulation 菜单能列出 CalculiX / OpenFOAM / demo solver
14. CalculiX 能导出 .inp
15. CalculiX 能调用 ccx
16. 能读取 .sta / .dat / .frd / .log
17. 能生成 ResultObject
18. Results 节点能展示结果
19. Properties 能展示结果详情
20. OpenFOAM placeholder 不影响 CalculiX
```

## 11. 常见问题诊断

### 11.1 Gmsh 找不到

现象：

```text
Gmsh environment check failed
```

检查：

```text
CMakeLists.txt -> MYCAE_GMSH_EXECUTABLE
gmsh.exe 是否存在
构建后是否使用了新的 CMake 配置
```

### 11.2 生成网格成功但没有物理组

现象：

```text
Gmsh physical groups and local mesh rules were not written
```

原因：

```text
当前 OCC face index -> Gmsh surface tag 稳定映射尚未完整实现。
```

影响：

```text
基础网格仍可生成；
但 CalculiX 边界条件如果依赖 MeshBoundary / PhysicalGroup，可能无法完成边界映射。
```

### 11.3 CalculiX 导出失败：没有 mesh

现象：

```text
CalculiX case data build failed: simulation case has no mesh.
```

处理：

```text
先选择几何并执行 Mesh -> Generate Mesh；
确认 Project / Model -> Mesh 下存在 mesh 对象；
```

### 11.4 CalculiX 导出失败：材料错误

现象：

```text
CalculiX material must be solid
CalculiX material is missing positive youngModulus
CalculiX material has invalid poissonRatio
```

处理：

```text
材料 Domain 选择 Solid；
Young Modulus > 0；
0 <= Poisson Ratio < 0.5；
```

### 11.5 CalculiX 导出失败：边界无法映射

现象：

```text
CalculiX export failed: no mesh physical group is available for boundary ...
```

原因：

```text
CalculiX 需要把 BoundaryCondition 映射到 NSET / ELSET / SURFACE。
当前需要 mesh 中有可识别的 2D physical group 或 MeshBoundary。
```

处理：

```text
短期：使用带 Physical Surface 的外部 .msh 测试 CalculiX；
后续：完善 FaceGroup 到 Gmsh PhysicalGroup 的稳定导出。
```

### 11.6 CalculiX 找不到 ccx

现象：

```text
CalculiX run failed: ccx executable does not exist
CalculiX run failed to start
```

处理：

```text
设置 MYCAE_CALCULIX_EXECUTABLE；
或把 ccx.exe 加入 PATH；
或在 CMake 中设置 MYCAE_CALCULIX_EXECUTABLE；
```

### 11.7 CalculiX 运行后没有结果文件

现象：

```text
CalculiX finished but no .sta, .dat or .frd result file was found.
```

检查：

```text
case 目录下 .inp 是否存在；
.log 中 stdout/stderr；
ccx 是否真实执行；
jobName 是否和 .inp 文件名一致；
```

## 12. 推荐测试顺序

如果只想快速验证一遍，按下面顺序：

```text
1. File -> New Project
2. Geometry -> Create Box
3. 选择 Box
4. Mesh -> Check Gmsh
5. Mesh -> Generate Mesh
6. Mesh -> Read Mesh Info
7. Mesh -> Show Mesh
8. Solver Setup -> Create Material，选择 Solid / Steel
9. Picking -> Pick Face
10. Picking -> Create Face Group from Pick
11. Solver Setup -> Create Boundary Condition
12. Solver Setup -> Create Load
13. Simulation -> Run CalculiX
14. 查看 Log、case 目录、Results 节点、Properties
15. 确认 Simulation 菜单中 OpenFOAM 是 reserved
```

这条线能覆盖当前项目的主要模块和接口边界。
