# MyCAE 轻量级 CAE 集成系统项目说明文档

## 1. 项目概述

MyCAE 是一个基于 Qt 6 开发的轻量级 CAE 集成系统，面向几何建模、网格划分、结构求解和结果后处理等基础仿真流程。系统集成 Open CASCADE、Gmsh、CalculiX 和 VTK 等开源组件，目标是实现从几何模型创建到仿真结果显示的完整工程闭环。

传统 CAE 软件通常功能庞大、学习成本较高，且内部流程不易观察。MyCAE 采用模块化方式实现基础 CAE 流程，使用户能够清楚理解几何、网格、工况、求解和后处理之间的数据流关系。该系统既可以作为轻量级 CAE 软件原型，也可以作为后续集成 OpenFOAM、AI Agent 自动化建模和多求解器调度的基础平台。

本项目当前已实现以下主线能力：

- 基于 Qt 6 的桌面软件界面；
- 基于 Open CASCADE 的基础几何创建、导入、布尔和变换；
- 基于 Gmsh 的三维网格生成和网格质量检查；
- 基于 CalculiX 的结构静力分析算例导出、求解和结果读取；
- 基于 VTK 的几何、网格和结果云图显示；
- 材料、边界条件、载荷和面组等仿真数据管理；
- 结果后处理、极值显示、点选查询、CSV 和报告导出；

## 2. 系统功能

### 2.1 工程管理功能

系统支持创建、打开和维护 MyCAE 工程。工程数据以目录形式组织，主要包括几何文件、网格文件、求解工况文件、结果文件和日志文件。工程入口文件为 `project.json`，用于记录工程名称、根目录和版本等基础信息。

工程管理模块的主要功能包括：

- 创建新工程目录；
- 打开已有工程；
- 管理几何、网格、求解和结果资源；
- 维护最近打开工程记录；
- 在工程树中展示模型数据；
- 保存仿真工况和结果记录。

### 2.2 几何建模功能

几何模块用于创建和管理仿真对象的几何模型。系统当前支持基础参数化几何和外部 STEP 文件导入。

主要功能包括：

- 创建长方体、圆柱体和球体；
- 导入 STEP 几何文件；
- 执行几何布尔操作；
- 对几何对象进行平移、旋转和缩放；
- 显示几何边线；
- 保存几何元数据和 OCC 几何文件；
- 维护几何对象与后续网格、边界条件之间的关联。

几何数据主要保存为三类文件：

```text
geometry/*.json   MyCAE 几何元数据
geometry/*.brep   Open CASCADE 原生几何文件
geometry/*.step   用于网格划分和外部交换的 STEP 文件
```

### 2.3 面组与拾取功能

面组用于把几何模型上的若干面组织成可命名对象。它是几何模型和仿真边界条件之间的重要连接层。

主要功能包括：

- 在 VTK 显示窗口中拾取几何面；
- 从拾取结果创建面组；
- 向已有面组添加或移除面；
- 重命名、删除和清空面组；
- 为面组设置局部网格尺寸；
- 将面组标记为 Gmsh Physical Surface；
- 将面组作为边界条件或载荷的目标。

面组设计的意义在于避免边界条件直接绑定到不稳定的 UI 选择状态，而是绑定到可持久化、可复查的几何引用。

### 2.4 网格划分功能

网格模块负责将几何模型转换为有限元计算所需的网格数据。系统通过 Gmsh 调用外部网格划分程序，并读取生成的 `.msh` 文件。

主要功能包括：

- 检查 Gmsh 环境；
- 根据 STEP/BREP 几何生成三维四面体网格；
- 支持一阶四面体和二阶四面体设置；
- 支持自动网格尺寸和手动网格尺寸；
- 支持基于面组的局部网格控制；
- 读取 MSH 2 格式文件；
- 统计节点数、单元数和表面三角形数量；
- 进行网格质量检查；
- 将网格转换为 VTK UnstructuredGrid 进行显示；
- 从 Physical Group 恢复网格边界。

网格阶段的典型流程为：

```text
STEP/BREP 几何
-> Gmsh 输入
-> gmsh.exe 生成 .msh
-> MshReader 读取网格
-> MeshData 保存节点和单元
-> MeshBoundaryBuilder 恢复边界
-> MeshToVtkConverter 转换为 VTK 网格
```

### 2.5 仿真数据定义功能

仿真数据模块用于描述求解器运行所需的材料、截面分配、边界条件和载荷。

当前支持的数据包括：

- 结构材料；
- 流体材料；
- 材料分区；
- 结构约束；
- 结构载荷；
- CFD 边界；
- CFD 场值；
- 求解控制参数。

其中结构分析主要面向 CalculiX，CFD 数据结构为后续 OpenFOAM 扩展预留。系统将仿真数据统一保存到 `solver/case.json`，由 `SimulationCaseBuilder` 在运行求解时组装成标准工况对象。

### 2.6 CalculiX 求解功能

CalculiX 是本项目当前主要实现的结构求解器。系统通过插件接口调用 CalculiX 相关模块，实现输入文件生成、外部进程执行和结果读取。

主要功能包括：

- 根据工程数据生成 CalculiX 输入文件 `.inp`；
- 将网格节点和单元写入输入 deck；
- 写入材料、单元集合、节点集合和表面集合；
- 写入边界条件和载荷；
- 调用 `ccx.exe` 执行结构静力求解；
- 读取 `.dat`、`.sta`、`.frd` 和日志文件；
- 生成 `ResultObject` 结果记录；
- 在工程树和结果面板中展示求解结果。

CalculiX 求解流程为：

```text
ProjectModel
-> SimulationCase
-> CalculiXCaseData
-> CalculiXInputDeck
-> ccx -i jobName
-> .dat / .sta / .frd / log
-> CalculiXResultReader
-> ResultObject
```

### 2.7 结果显示与后处理功能

结果模块用于读取求解结果并进行可视化展示。系统通过 VTK 显示网格和云图，同时在右侧后处理面板中展示结果字段、极值和点选信息。

主要功能包括：

- 读取 CalculiX 结果数据；
- 显示位移云图；
- 显示 Von Mises 应力云图；
- 支持变形比例调整；
- 支持显示网格边；
- 支持未变形轮廓叠加；
- 支持色标范围锁定；
- 支持结果点选查询；
- 计算并标记最大值和最小值；
- 导出 CSV 数据；
- 导出结果报告；
- 导出渲染截图。

当前典型结果字段包括：

```text
Ux
Uy
Uz
Displacement Magnitude
Von Mises Stress
```

## 3. 系统架构与程序设计

### 3.1 总体架构

MyCAE 采用轻量分层架构，将界面、流程控制、领域数据、第三方库适配和外部进程调用进行分离。

总体结构如下：

```text
Qt UI 界面层
  -> Workflow / Controller 流程控制层
  -> ProjectModel / Repository / Service 数据管理层
  -> OCC / Gmsh / CalculiX / VTK 适配层
  -> 文件系统和外部求解器
```

各层职责如下：

- UI 层负责窗口、菜单、工具栏、Dock 面板和用户交互；
- Workflow 层负责将用户操作编排为完整业务流程；
- ProjectModel 和 Repository 层负责保存运行时数据；
- Manager 和 Service 层负责领域操作、文件读写和数据转换；
- 适配层负责调用 Open CASCADE、Gmsh、CalculiX 和 VTK；
- 文件系统和外部进程层负责实际数据持久化和求解器执行。

这种设计可以避免 UI 直接读写复杂文件格式，也便于后续替换求解器、增加新模块或接入自动化 Agent。

### 3.2 目录结构设计

项目主要代码位于 `src/` 目录下，各模块职责如下：

```text
src/
  commands/      命令封装和 undo/redo
  diagnostics/   诊断消息收集和分类
  geometry/      几何对象、面组、几何管理
  mesh/          Gmsh 调用、MSH 读取、网格质量检查
  occ/           Open CASCADE 几何构造、布尔、IO 和转换
  picking/       拾取模式、拾取状态和拾取控制
  project/       工程模型、Repository 和工程加载
  render/        VTK 高亮、拾取适配和几何辅助
  result/        结果加载、结果显示、极值计算和导出
  solver/        工况数据、求解器插件、CalculiX/OpenFOAM 适配
  ui/            Qt 主窗口、面板、对话框和样式
  validation/    样例验证、UI 验证和截图采集
  workflow/      面向用户操作的流程控制器
```

该目录划分基本对应 CAE 软件的核心业务流程，便于学习和定位问题。

### 3.3 MainWindow 程序设计

`MainWindow` 是系统主窗口，但它不直接实现大量业务逻辑，而是负责组合各类 UI 组件和高层控制器。

当前主窗口已拆分为多个协作对象：

```text
MainWindow
  MainWindowActions
  MainWindowMenuBuilder
  MainWindowToolBarBuilder
  MainWindowDocks
  MainWindowStateController
  MainWindowResultController
  MainWindowToolController
  MainWindowViewController
  MainWindowLifecycleController
  SelectionInteractionController
  RecentProjectController
```

这种拆分方式使主窗口主要承担以下职责：

- 创建和组织 UI 组件；
- 连接高层信号槽；
- 持有核心上下文对象；
- 将用户操作转发给 workflow 或 controller；
- 管理窗口生命周期和自动化关闭流程。

这种设计比把所有逻辑写在 `MainWindow.cpp` 中更利于维护，也更适合后续继续扩展功能。

### 3.4 ProjectModel 与 Repository 设计

`ProjectModel` 是系统运行时数据的核心入口，用于聚合工程状态和各类仓储对象。

核心结构如下：

```text
ProjectModel
  ProjectContext
  GeometryRepository
  MeshRepository
  SolverRepository
  ResultRepository
  SelectionState
```

其中：

- `GeometryRepository` 保存几何对象；
- `MeshRepository` 保存网格对象和网格设置；
- `SolverRepository` 保存材料、边界条件、载荷、面组和工况；
- `ResultRepository` 保存求解结果记录；
- `SelectionState` 保存当前工程树或视图中的选择状态。

UI 和 workflow 通过 `ProjectModel` 访问数据，具体文件读写交由 `ProjectManager`、`GeometryManager`、`MeshManager`、`SimulationCaseManager` 等类完成。

### 3.5 Workflow 流程控制设计

Workflow 层是连接用户操作和底层模块的中间层。它负责把多个低层操作组合成一个完整的用户动作。

典型控制器包括：

```text
ProjectWorkflowController
GeometryWorkflowController
MeshWorkflowController
SolverWorkflowController
SolverCaseWorkflowController
ResultWorkflowController
SelectionController
```

例如用户点击“运行 CalculiX”时，系统内部流程如下：

```text
用户点击运行按钮
-> SolverWorkflowController
-> SolverCaseWorkflowController
-> 保存当前 SimulationCase
-> 查找 SolverPlugin
-> 导出求解算例
-> 调用求解器运行
-> 读取求解结果
-> 创建 ResultObject
-> 刷新工程树和结果面板
```

Workflow 层的存在使 UI 不必直接了解 CalculiX 输入文件、结果文件或求解器命令细节，从而降低界面层和求解层之间的耦合。

### 3.6 求解器插件设计

系统通过 `SolverPlugin` 接口抽象求解器能力。每个求解器插件需要实现三个核心动作：

```text
exportCase
runCase
readResult
```

其含义分别是：

- `exportCase`：根据工程数据导出求解器算例；
- `runCase`：调用外部求解器或远程服务运行算例；
- `readResult`：读取求解完成后的结果文件。

当前已包含：

- `CalculiXPlugin`：结构有限元求解插件；
- `OpenFoamPlugin`：OpenFOAM demo service 插件；
- `ExternalProcessSolverPlugin`：外部进程插件示例。

插件化设计的优势是后续可以继续扩展不同求解器，而不需要重写 UI 和 workflow 主流程。

### 3.7 结果显示设计

结果显示模块采用“结果对象 + 数据加载 + VTK 显示控制”的方式组织。

主要数据流如下：

```text
ResultObject
-> ResultDataLoader
-> MeshData + ResultData
-> ResultDisplayController
-> ResultExtremaCalculator
-> RenderView / VtkRenderCanvas
-> ResultPostprocessPanel
```

其中 `ResultDisplayController` 负责决定显示字段、标量范围、变形比例、网格边和未变形轮廓等参数；`VtkRenderCanvas` 负责具体 VTK 场景渲染；`ResultPostprocessPanel` 负责在界面中展示后处理控制和结果摘要。

## 4. 关键技术与项目亮点

### 4.1 Qt 多面板桌面软件设计

系统采用 Qt Widgets 构建桌面端工程软件界面，主界面包含菜单栏、工具栏、工程树、属性面板、三维视图、诊断面板、日志面板和结果后处理面板。整体风格接近传统工程软件，方便用户按照 CAE 流程逐步操作。

界面设计中使用 Dock 面板组织功能区域，使工程树、属性查看、后处理控制和日志输出相互独立，便于用户在不同阶段关注不同信息。

### 4.2 Open CASCADE 几何建模集成

系统使用 Open CASCADE 构建几何对象，并支持 BREP 和 STEP 文件导出。基础几何对象由参数驱动生成，导出的 STEP 文件可以作为 Gmsh 网格划分的输入。

该设计使系统具备基础几何建模能力，同时保留与其他 CAD/CAE 工具交换数据的可能。

### 4.3 Gmsh 自动网格划分

网格模块通过 `QProcess` 调用 `gmsh.exe`，将几何文件转换为有限元网格。系统不仅生成网格，还会读取网格数据、统计节点和单元数量，并进行网格质量检查。

局部网格控制和 Physical Group 的引入，使面组能够在网格阶段继续保持语义，从而为后续边界条件映射提供依据。

### 4.4 CalculiX 求解器集成

系统实现了从工程数据到 CalculiX 输入 deck 的转换，并能够调用 `ccx.exe` 运行求解。输入文件包含节点、单元、材料、集合、边界条件、载荷和分析步等内容。

该模块体现了 CAE 软件中“前处理到求解器”的关键技术路线，是本项目实现仿真闭环的核心部分。

### 4.5 VTK 三维可视化与后处理

系统使用 VTK 显示几何、网格和求解结果。结果显示支持云图、色标、网格边、变形比例、极值标记和点选查询。

VTK 的引入使系统不仅能生成求解结果，还能以工程软件常见的三维方式展示仿真数据，提高了软件完整性和可用性。

### 4.6 求解器插件化架构

系统没有把 CalculiX 写死在 UI 中，而是通过 `SolverPlugin` 抽象求解器能力。该设计使求解器运行流程统一为：

```text
导出算例 -> 运行求解器 -> 读取结果
```

这种结构适合后续扩展 OpenFOAM、Code_Aster、Elmer 或自定义 Python 求解器，也适合与 AI Agent 自动化流程结合。

## 5. 系统实施方案

### 5.1 开发路线

本项目采用分阶段、分模块、可运行优先的开发方式。整体实施顺序如下：

```text
项目骨架搭建
-> Qt 主界面和 Dock 面板
-> 工程数据模型
-> 几何创建和 OCC 导出
-> 面组和拾取
-> Gmsh 网格生成
-> 仿真数据定义
-> CalculiX 算例导出
-> 求解器运行和结果读取
-> VTK 结果显示
-> 后处理功能
-> 自动化验证
```

这种顺序保证每一阶段都有可运行成果，而不是一次性堆叠大量无法验证的代码。

### 5.2 几何实施方案

几何阶段首先定义长方体、圆柱体、球体等参数化数据结构，然后通过 Open CASCADE 创建对应的 `TopoDS_Shape`。创建完成后，系统将几何保存为 JSON 元数据，并导出 BREP 和 STEP 文件。

几何创建流程如下：

```text
用户输入几何参数
-> GeometryCreationController
-> GeometryManager
-> OCCGeometryFactory
-> OCCShapeIO
-> 保存 JSON / BREP / STEP
-> ProjectModel 更新几何仓储
-> RenderView 显示几何
```

该流程将 UI 输入、领域管理、OCC 调用和显示刷新分离，便于后续增加更多几何类型。

### 5.3 网格实施方案

网格阶段以几何 STEP 文件为输入，由 Gmsh 生成 `.msh` 文件。系统读取 `.msh` 后构建内部 `MeshData`，再生成 `MeshObject` 保存到工程中。

流程如下：

```text
选择几何对象
-> 配置网格参数
-> GmshCaseWriter 准备输入
-> GmshRunner 调用 gmsh.exe
-> MshReader 读取 .msh
-> MeshQualityChecker 检查质量
-> MeshBoundaryBuilder 构建边界
-> MeshManager 保存网格对象
-> VTK 显示网格
```

该方案的关键点是网格不仅作为显示数据存在，还要保留边界信息，用于后续求解器边界条件映射。

### 5.4 求解实施方案

求解阶段通过 `SimulationCaseBuilder` 将工程中的材料、边界条件、载荷、网格和面组组装成统一工况，再交给求解器插件处理。

CalculiX 求解流程如下：

```text
ProjectModel
-> SimulationCaseBuilder
-> CalculiXCaseDataBuilder
-> CalculiXInputDeckBuilder
-> CalculiXCaseWriter
-> CalculiXRunner
-> CalculiXResultReader
-> ResultRepository
```

求解结果会被包装成 `ResultObject`，并保存到工程结果仓储中。用户可以在工程树中选择结果对象，再由结果显示模块进行后处理。

### 5.5 后处理实施方案

后处理阶段从 `ResultObject` 出发，读取结果文件和网格文件，生成可显示的 VTK 网格。系统根据用户选择的字段显示云图，并计算对应的标量范围和极值。

流程如下：

```text
选择 ResultObject
-> ResultDataLoader 加载数据
-> ResultDisplayController 选择字段
-> CalculiXResultGridBuilder 构建结果网格
-> ResultExtremaCalculator 计算极值
-> VtkRenderCanvas 显示云图
-> ResultPostprocessPanel 更新控制面板
```

## 6. 系统部署与运行介绍

### 6.1 开发环境

本项目当前主要面向 Windows 桌面环境开发和运行。

推荐环境如下：

```text
操作系统：Windows 10 / Windows 11
开发语言：C++17
界面框架：Qt 6.6.x
构建工具：CMake 3.21 及以上
编译器：MSVC 64-bit
几何内核：Open CASCADE
可视化库：VTK
网格工具：Gmsh
结构求解器：CalculiX
```

### 6.2 第三方依赖

系统运行依赖以下第三方组件：

- Qt：提供桌面界面、信号槽、文件和进程管理；
- Open CASCADE：提供几何建模、STEP/BREP 读写；
- VTK：提供三维渲染、网格和结果显示；
- Gmsh：提供三维网格划分能力；
- CalculiX：提供结构有限元求解能力；
- CMake：提供工程构建和测试入口。

### 6.3 构建命令

在工程目录下执行：

```powershell
cmake --build .\build-codex-qt663-ninja --target MyCAE
```

如果首次配置工程，需要先使用 CMake 指定 Qt、VTK、Open CASCADE 等库路径。当前项目中 `CMakeLists.txt` 已经提供本机路径示例，实际部署到其他电脑时，应根据本机安装位置修改对应路径或使用 CMake Cache 参数指定。

### 6.4 运行程序

构建完成后，运行：

```powershell
.\build-codex-qt663-ninja\MyCAE.exe
```

正常启动后应看到 MyCAE 主界面，左侧为工程树，中间为三维显示区域，右侧为属性和结果后处理面板，下方包含日志和诊断信息。

## 7. 程序截图

本节用于展示系统主要运行界面。当前项目已经提供自动化截图采集命令，截图输出目录为：

```text
build-codex-qt663-ninja/ui_screenshots/
```

### 7.1 结果属性界面

该截图展示打开样例工程后，工程树、属性面板和结果信息的显示效果。

![结果属性界面](build-codex-qt663-ninja/ui_screenshots/box_pressure_demo_properties.png)

### 7.2 结果后处理界面

该截图展示结果后处理面板，包括结果字段、变形比例、网格边显示、极值信息和导出按钮等内容。

![结果后处理界面](build-codex-qt663-ninja/ui_screenshots/box_pressure_demo_result_postprocess.png)

### 7.3 建议补充截图

为了让文档更加完整，正式提交 Word 或 PDF 时建议继续补充以下截图：

1. 软件启动后的主界面；
2. 创建长方体、圆柱体或球体的参数输入界面；
3. 几何模型显示界面；
4. 面拾取和面组创建界面；
5. Gmsh 网格生成后的网格显示界面；
6. 材料、边界条件和载荷定义界面；
7. CalculiX 求解完成后的日志界面；
8. VTK 结果云图和极值标记界面；
9. CSV 或报告导出结果界面。

截图应按照“建模 -> 网格 -> 求解 -> 后处理”的顺序排列，每张图下方附一到两句说明，突出该步骤在完整 CAE 流程中的作用。
