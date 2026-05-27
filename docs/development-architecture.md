# MyCAE 开发架构说明

本文档面向开发者，说明当前代码结构、模块职责和后续扩展方向。

## 1. 架构目标

MyCAE 当前采用轻量分层架构：

```text
UI 层
  -> Workflow / Controller 层
  -> Domain / Repository / Service 层
  -> Solver / Mesh / OCC / VTK 适配层
  -> 文件系统和外部进程
```

核心原则：

- UI 不直接操作复杂文件格式
- Workflow 负责用户动作编排
- Repository 保存运行时数据
- Manager/Service 负责持久化和领域操作
- SolverPlugin 隔离不同求解器
- VTK/OCC/Gmsh/CalculiX 作为适配层，不污染 UI 逻辑

## 2. 目录结构

```text
src/
  commands/      命令和 undo/redo
  diagnostics/   诊断消息
  geometry/      几何、面组、几何管理
  mesh/          Gmsh、MSH 读取、网格边界、VTK 网格转换
  occ/           Open CASCADE 构造、布尔、IO、VTK 转换
  picking/       面拾取状态和拾取控制
  project/       ProjectModel、Repository、工程加载
  render/        VTK 高亮、拾取适配、几何辅助
  result/        结果加载、摘要、极值、导出
  solver/        工况、求解器插件、CalculiX/OpenFOAM 适配
  ui/            Qt 界面、Dock、Panel、Dialog
  validation/    自动化验收和截图
  workflow/      面向用户操作的流程控制器
```

## 3. MainWindow 拆分

`MainWindow` 已经拆成多个协作对象：

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

`MainWindow` 的职责应保持为：

- 组合 UI 组件
- 连接高层信号槽
- 持有核心上下文
- 转发用户动作到 controller

不应该再把复杂业务逻辑写回 `MainWindow.cpp`。

## 4. ProjectModel 和 Repository

关键文件：

```text
src/project/ProjectModel.*
src/project/GeometryRepository.*
src/project/MeshRepository.*
src/project/SolverRepository.*
src/project/ResultRepository.*
src/project/SelectionState.*
```

`ProjectModel` 聚合运行时状态：

```text
Project
GeometryRepository
MeshRepository
SolverRepository
ResultRepository
SelectionState
```

设计建议：

- 新增领域对象时，优先放入对应 repository
- UI 查询数据时通过 `ProjectModel`
- 文件读写不要直接写在 UI 类中

## 5. Workflow 层

关键文件：

```text
src/workflow/ProjectWorkflowController.*
src/workflow/GeometryWorkflowController.*
src/workflow/MeshWorkflowController.*
src/workflow/SolverCaseWorkflowController.*
src/workflow/SolverWorkflowController.*
src/workflow/ResultWorkflowController.*
src/workflow/SelectionController.*
```

Workflow 层负责把多个低层操作组合成用户动作。

示例：

```text
用户点击“运行 CalculiX”
-> SolverWorkflowController
-> SolverPluginManager
-> CalculiXPlugin
-> 导出 / 运行 / 读取结果
-> ProjectModel 增加 ResultObject
-> UI 刷新树和面板
```

设计建议：

- Workflow 可以访问 UI 面板，但不应实现复杂 UI 布局
- Workflow 可以调用 manager/service/plugin
- Workflow 返回 result/logMessages，便于日志和自动化测试

## 6. SolverPlugin 架构

关键接口：

```text
src/solver/plugin/SolverPlugin.h
```

插件基本能力：

```text
export
run
read-result
```

当前内置插件：

```text
CalculiXPlugin
OpenFoamPlugin
```

外部插件：

```text
resource/solver/demo_solver
```

扩展新求解器时应优先实现：

```text
SolverPlugin
SolverCaseWriter
SolverRunResult
SolverResultReadResult
```

不要让 UI 层直接感知具体求解器文件格式。

## 7. Result 模块

关键文件：

```text
src/result/ResultDataLoader.*
src/result/ResultDisplayController.*
src/result/ResultDisplaySummary.*
src/result/ResultExtremaCalculator.*
src/result/ResultCsvExporter.*
src/result/ResultReportExporter.*
```

当前职责划分：

- `ResultDataLoader`：读取网格和 CalculiX `.dat`
- `ResultDisplaySummary`：计算轻量覆盖率和标量范围
- `ResultDisplayController`：决定摘要显示或 VTK 显示
- `ResultExtremaCalculator`：计算极值和标记
- `ResultCsvExporter`：导出 CSV
- `ResultReportExporter`：导出报告

自动化验收使用摘要模式，正常交互使用完整 VTK 显示。

## 8. UI 模块

面板和工具类：

```text
src/ui/ProjectTreePanel.*
src/ui/PropertyPanel.*
src/ui/ResultPostprocessPanel.*
src/ui/ResultPostprocessText.*
src/ui/DiagnosticPanel.*
src/ui/LogPanel.*
src/ui/RenderView.*
src/ui/VtkRenderCanvas.*
```

当前 UI 设计要求：

- 主风格保持白色、清晰、工程软件风格
- 菜单保持紧凑
- 工具栏使用 icon-only
- 属性和结果后处理面板使用分组
- 右侧面板必须支持滚动，避免最大化或小窗口时内容被截断

## 9. Validation 模块

关键文件：

```text
src/validation/SampleProjectValidator.*
src/validation/DemoProjectValidator.*
src/validation/UiSmokeValidator.*
src/validation/UiSampleScreenshotter.*
src/validation/UiAutomationSupport.*
```

验收命令：

```powershell
.\MyCAE.exe --validate-samples
.\MyCAE.exe --validate-ui
.\MyCAE.exe --capture-ui-sample --ui-screenshot-dir .\ui_screenshots
```

CTest：

```powershell
ctest --test-dir .\build-codex-qt663-ninja --output-on-failure
```

新增 UI 功能后，应同步补充 `UiSmokeValidator` 中的控件存在性、启用状态或真实样例状态检查。

## 10. 继续重构建议

优先级从高到低：

1. 拆 `GeometryManager`
   - 几何 JSON 编解码
   - 文件命名
   - OCC 导出
   - 几何仓储加载

2. 拆 `VtkRenderCanvas`
   - 场景状态
   - 几何显示
   - 网格显示
   - 结果显示
   - 拾取和高亮

3. 拆 `ResultWorkflowController`
   - 导出 CSV
   - 导出报告
   - 截图
   - 结果历史操作

4. 引入 Qt 翻译体系
   - `tr()`
   - `.ts/.qm`
   - 中英文切换

5. OpenFOAM 插件实装
   - case writer
   - mesh workflow
   - solver runner
   - VTK/result reader

## 11. 开发约定

新增功能建议遵循：

```text
数据结构
-> manager/service
-> workflow/controller
-> UI panel/dialog
-> validation
```

不要先从 UI 写起，也不要让 UI 直接读写复杂文件。

每次较大改动后至少运行：

```powershell
cmake --build .\build-codex-qt663-ninja --target MyCAE
ctest --test-dir .\build-codex-qt663-ninja --output-on-failure
```
