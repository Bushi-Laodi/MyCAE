# MyCAE CalculiX 悬臂梁仿真测试用例

本文档用于测试当前 MyCAE 的完整结构仿真链路：

```text
Box 几何
  -> FaceGroup
  -> 材料 / 约束 / 载荷
  -> Gmsh Physical Surface 网格
  -> CalculiX .inp
  -> ccx 求解
  -> .sta / .dat / .frd
  -> ResultObject
```

## 1. 测试目标

使用一个长方体悬臂梁作为最小结构仿真算例：

```text
左端固定；
右端施加压力；
材料为钢；
使用 CalculiX 求解静力结构响应。
```

该用例重点验证：

```text
1. FaceGroup 能绑定到几何面
2. FaceGroup 能导出为 Gmsh Physical Surface
3. MeshBoundary 能从 .msh 中恢复
4. CalculiX 能生成 NSET / ELSET / SURFACE
5. CalculiX 能导出并运行 .inp
6. 结果能进入 Results 节点
```

## 2. 环境准备

### 2.1 Gmsh

确认 `Mesh -> Check Gmsh` 成功。

期望日志：

```text
Gmsh environment check succeeded.
```

### 2.2 CalculiX

确认已经设置：

```powershell
setx MYCAE_CALCULIX_EXECUTABLE "D:\cpp_lib\calculix\CalculiX-2.23.0-win-x64\bin\ccx.exe"
```

设置后需要重新打开：

```text
MyCAE
Qt Creator
```

如果未配置 CalculiX，本用例仍可测试到 `.inp` 导出阶段，但不能完成求解。

## 3. 新建项目

菜单：

```text
File -> New Project
```

建议项目目录：

```text
C:\Users\li\Desktop\mycae_cantilever_test
```

期望日志：

```text
Project created: ...
Simulation case saved: solver/case.json
```

## 4. 创建悬臂梁几何

菜单：

```text
Geometry -> Create Box
```

参数：

```text
Length = 100 mm
Width  = 20 mm
Height = 20 mm
Unit   = mm
```

期望：

```text
Project / Model -> Geometry 下出现 Box_1
生成 geometry/box_1.brep
生成 geometry/box_1.step
RenderView 显示长方体
```

## 5. 创建两个 FaceGroup

必须先创建两个 FaceGroup，再生成网格。这样 Gmsh 才能把它们写成 Physical Surface。

### 5.1 创建固定端 FaceGroup

1. 在项目树中选择 `Box_1`
2. 菜单：

```text
Picking -> Pick Face
```

3. 点击长方体左端面
4. 菜单：

```text
Picking -> Create Face Group from Pick
```

5. 名称输入：

```text
FixedEnd
```

期望：

```text
Face group created: Box_1.FixedEnd with 1 face(s).
```

### 5.2 创建受力端 FaceGroup

1. 保持 `Pick Face` 模式
2. 点击长方体右端面
3. 菜单：

```text
Picking -> Create Face Group from Pick
```

4. 名称输入：

```text
LoadEnd
```

期望：

```text
Face group created: Box_1.LoadEnd with 1 face(s).
```

## 6. 创建材料

菜单：

```text
Solver Setup -> Create Material
```

参数：

```text
Name            = Steel
Domain          = Solid
Enable Density  = true
Density         = 7800 kg/m^3
Young Modulus   = 2.1e11 Pa
Poisson Ratio   = 0.3
```

期望：

```text
Material created: Steel (ID: steel)
```

## 7. 创建边界条件

### 7.1 固定端约束

菜单：

```text
Solver Setup -> Create Boundary Condition
```

参数：

```text
Name        = FixedEnd
Type        = Wall
Geometry    = Box_1
Face Group  = Box_1.FixedEnd
Material ID = steel
```

期望：

```text
Boundary condition created: FixedEnd (Type: wall)
```

说明：

```text
当前 CalculiX 导出逻辑中，Wall 且没有被载荷引用的边界会作为固定约束导出。
```

### 7.2 受力端边界

菜单：

```text
Solver Setup -> Create Boundary Condition
```

参数：

```text
Name        = LoadEndBc
Type        = Wall
Geometry    = Box_1
Face Group  = Box_1.LoadEnd
Material ID = steel
```

期望：

```text
Boundary condition created: LoadEndBc (Type: wall)
```

## 8. 创建载荷

菜单：

```text
Solver Setup -> Create Load
```

参数：

```text
Name                  = EndPressure
Type                  = Pressure
Boundary Condition ID = loadendbc
Field Name            = pressure
Value                 = 100000
Unit                  = Pa
```

期望：

```text
Load created: EndPressure (Value: 100000)
```

注意：

```text
Boundary Condition ID 必须填写边界条件 ID。
LoadEndBc 的 ID 是 loadendbc。
```

## 9. 生成网格

重新选择项目树中的：

```text
Box_1
```

菜单：

```text
Mesh -> Generate Mesh
```

期望日志应包含：

```text
Gmsh face group export scan: Box_1
FaceGroup export enabled: Box_1.FixedEnd
FaceGroup export enabled: Box_1.LoadEnd
Gmsh geo file written: .../geometry/box_1_physical.geo
Gmsh input: .../geometry/box_1_physical.geo
Mesh generated: mesh/box_1.msh
MeshObject saved: mesh/box_1_mesh.json
Mesh boundaries detected: 2
MeshBoundary: Box_1.FixedEnd
MeshBoundary: Box_1.LoadEnd
```

如果看到：

```text
Mesh boundaries detected: 0
```

说明你可能在创建边界条件之前就生成了网格。处理方式：

```text
重新选择 Box_1
再次执行 Mesh -> Generate Mesh
```

## 10. 检查网格

菜单：

```text
Mesh -> Read Mesh Info
```

期望：

```text
Read mesh succeeded.
Node count: > 0
Tetra count: > 0
```

菜单：

```text
Mesh -> Show Mesh
```

期望：

```text
RenderView 显示四面体网格
```

## 11. 运行 CalculiX

菜单：

```text
Simulation -> Run CalculiX
```

期望完整成功日志：

```text
Solver plugin: CalculiX (calculix)
Solver family: structural
Solver case directory: ...
CalculiX case data: ... nodes, ... tetrahedra, 1 material(s), 2 boundary(ies), 1 load(s).
CalculiX input deck build started.
CalculiX input deck build finished.
CalculiX input written: ... .inp
Solver input exported.
CalculiX command: D:\cpp_lib\calculix\CalculiX-2.23.0-win-x64\bin\ccx.exe -i <jobName>
CalculiX log: ... .log
CalculiX result directory: ...
Solver result: CalculiX result read: ...
Result index saved: result/results.json
```

期望生成文件：

```text
solver/calculix/<case_run>/
  <jobName>.inp
  <jobName>.log
  <jobName>.sta
  <jobName>.dat
  <jobName>.frd
```

## 12. 检查结果

在项目树中查看：

```text
Results
```

期望：

```text
出现 CalculiX Result
```

点击结果对象，右侧属性面板应显示：

```text
Solver      = CalculiX
Case Path   = solver/calculix/...
Log File    = ...
Success     = Yes
Summary     = ...
```

如果当前版本支持结果场显示，还应能看到：

```text
Ux
Uy
Uz
DisplacementMagnitude
VonMisesStress
```

## 13. 验收标准

本测试通过条件：

```text
1. Box 几何创建成功
2. FixedEnd / LoadEnd 两个 FaceGroup 创建成功
3. Steel 材料创建成功
4. FixedEnd / LoadEndBc 两个边界条件创建成功
5. EndPressure 载荷创建成功
6. Gmsh 使用 box_1_physical.geo 生成网格
7. Mesh boundaries detected 至少为 2
8. CalculiX .inp 导出成功
9. ccx 成功运行
10. .sta / .dat / .frd 至少生成一个结果文件
11. Results 中出现 CalculiX Result
```

## 14. 常见失败与处理

### 14.1 ccx 找不到

现象：

```text
CalculiX run failed to start
系统找不到指定的文件
```

处理：

```powershell
setx MYCAE_CALCULIX_EXECUTABLE "D:\cpp_lib\calculix\CalculiX-2.23.0-win-x64\bin\ccx.exe"
```

然后重启 MyCAE / Qt Creator。

### 14.2 Mesh boundaries detected: 0

原因：

```text
生成网格时，FaceGroup 没有被导出为 Physical Surface。
```

处理：

```text
确认 FixedEnd / LoadEnd FaceGroup 已创建；
确认 FixedEnd / LoadEndBc 边界条件已创建；
重新选择 Box_1；
重新 Mesh -> Generate Mesh。
```

### 14.3 no fixed boundary constraint was written

原因：

```text
没有可导出的固定约束。
```

处理：

```text
确认 FixedEnd 边界条件 Type = Wall；
确认 FixedEnd 没有被 Load 引用；
确认 FixedEnd 绑定的是 Box_1.FixedEnd FaceGroup。
```

### 14.4 load target boundary not found

原因：

```text
Load 的 Boundary Condition ID 填错。
```

处理：

```text
LoadEndBc 的 ID 是 loadendbc；
EndPressure 的 Boundary Condition ID 应填写 loadendbc。
```

## 15. 推荐最短测试顺序

```text
1. File -> New Project
2. Geometry -> Create Box
3. Pick Face -> Create FaceGroup: FixedEnd
4. Pick Face -> Create FaceGroup: LoadEnd
5. Create Material: Steel / Solid
6. Create Boundary Condition: FixedEnd / Wall / Box_1.FixedEnd
7. Create Boundary Condition: LoadEndBc / Wall / Box_1.LoadEnd
8. Create Load: EndPressure / Pressure / boundaryConditionId=loadendbc
9. Select Box_1
10. Mesh -> Generate Mesh
11. Mesh -> Show Mesh
12. Simulation -> Run CalculiX
13. Check Results
```
