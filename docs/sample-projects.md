# MyCAE 样例工程说明

样例用于快速验证软件主流程，不要求用户从零创建工程。

## 1. 样例目录

当前样例位于：

```text
samples/
  calculix/
    single_tetra.inp
    box_pressure.inp
  projects/
    box_pressure_demo/
      project.json
      README.md
      DEMO_CHECKLIST.md
      geometry/
      mesh/
      solver/
```

构建时样例会复制到可执行文件目录：

```text
build-codex-qt663-ninja/samples/
```

自动化验收优先使用构建目录中的样例；如果不存在，会回退到源码目录。

## 2. box_pressure_demo

这是当前最重要的真实样例工程。

### 2.1 工程内容

```text
几何：Box_1
网格：Box_1_Mesh
材料：Steel
边界条件：FixedEnd、LoadEndBc
载荷：EndPressure
结果：CalculiX Result - Box Pressure Demo
```

### 2.2 几何和网格

几何文件：

```text
geometry/box_1.json
geometry/box_1.brep
geometry/box_1.step
```

网格文件：

```text
mesh/box_1.msh
mesh/box_1_mesh.json
```

该网格包含可用于边界映射的表面分组。

### 2.3 工况和结果

工况文件：

```text
solver/case.json
```

结果索引：

```text
solver/results.json
```

结果目录中包含 CalculiX 输出文件：

```text
.dat
.sta
.frd
.log
```

MyCAE 主要从 `.dat` 读取位移和应力，从 `.sta/.dat/.log` 判断求解完成状态。

## 3. CalculiX 输入 deck 样例

### 3.1 single_tetra.inp

用途：

- 最小单元测试
- 验证 `ccx` 是否可运行
- 验证 `.dat/.sta/.frd` 是否能生成
- 验证位移和应力读取

适合排查 CalculiX 环境问题。

### 3.2 box_pressure.inp

用途：

- 验证稍复杂的结构输入 deck
- 验证压力载荷
- 验证 Von Mises 应力读取

适合排查结果读取和后处理字段。

## 4. 自动化样例验收

运行：

```powershell
.\MyCAE.exe --validate-samples
```

验收内容：

```text
1. 样例目录存在
2. box_pressure_demo 工程可打开
3. 几何、网格、材料、边界条件、载荷、结果数量正确
4. 样例资源文件完整
5. demo 结果数据可读取
6. CalculiX 可执行文件可找到
7. single_tetra.inp 可求解
8. box_pressure.inp 可求解
9. .dat/.sta/.frd 结果文件生成
10. 位移和 Von Mises 字段可读取
```

成功示例：

```text
MyCAE sample validation
Passed: 23
Failed: 0
```

## 5. UI 样例验收

运行：

```powershell
.\MyCAE.exe --validate-ui
```

覆盖内容：

- 菜单栏
- 工具栏
- Dock 面板
- 项目树
- 属性面板
- 结果后处理面板
- 真实样例工程打开
- 结果节点选择
- 结果字段、覆盖率、单位、导出按钮

成功示例：

```text
MyCAE UI validation
Passed: 128
Failed: 0
```

## 6. 样例截图

运行：

```powershell
.\MyCAE.exe --capture-ui-sample --ui-screenshot-dir .\ui_screenshots
```

输出：

```text
box_pressure_demo_properties.png
box_pressure_demo_result_postprocess.png
```

截图用于人工检查 UI 是否出现文字挤压、Dock 布局异常、表格行高不协调等问题。
