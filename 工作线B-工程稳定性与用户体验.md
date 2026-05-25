# 工作线 B：工程稳定性与用户体验

## 目标

把 MyCAE 从“功能能跑”推进到“稳定、好调试、好演示、可长期扩展”。

当前基础已经具备：

- 项目、几何、网格、求解、后处理主流程已经串通；
- 日志面板能显示运行过程；
- 结果树和后处理面板已经存在；
- CalculiX 环境、运行、读取结果已经基本稳定。

本工作线重点提升软件工程质量和用户体验。

## 模块边界

主要涉及：

- `src/workflow/`
- `src/project/`
- `src/ui/`
- `src/commands/`
- `samples/`
- `resource/`

尽量不修改：

- CalculiX 数值结果解析核心；
- VTK 结果云图算法；
- 几何布尔和底层网格算法。

## 阶段 B1：错误诊断中心

### 目标

把日志里的 warning/error 分类成明确诊断项，而不是全部堆在 LogPanel 里。

诊断类型建议：

- Environment：环境错误，例如 CalculiX/Gmsh 路径不存在；
- Input：输入数据错误，例如没有材料、没有载荷；
- Mesh：网格错误，例如 MSH 缺失、节点/单元为空；
- Solver：求解错误，例如 CalculiX failed；
- Result：结果错误，例如 `.dat/.frd/.sta` 缺失；
- UI：界面操作错误，例如未选择对象。

### 建议新增/修改文件

- 新增 `src/diagnostics/DiagnosticMessage.h`
- 新增 `src/diagnostics/DiagnosticCollector.cpp`
- 新增 `src/diagnostics/DiagnosticCollector.h`
- 新增 `src/ui/DiagnosticPanel.cpp`
- 新增 `src/ui/DiagnosticPanel.h`
- 修改 `src/ui/MainWindow.*`
- 修改关键 workflow，把重要错误写入诊断中心。

### UI 建议

新增右侧或底部 dock：

```text
Diagnostics
```

列：

- Severity；
- Category；
- Message；
- Suggested Fix。

### 验收标准

- CalculiX 路径错误能显示为 Environment；
- MSH 缺失能显示为 Mesh；
- 结果文件缺失能显示为 Result；
- 每条诊断都有简短修复建议。

## 阶段 B2：样例工程自动验证

### 目标

内置可重复验证的样例工程，确保以后改代码不会把主流程改坏。

建议样例：

1. `single_tetra`
   - 最小 CalculiX smoke test；
   - 验证 ccx 能运行；
   - 验证 `.dat/.sta/.frd` 文件生成。

2. `box_pressure`
   - 当前主流程样例；
   - box 几何；
   - tetra mesh；
   - fixed boundary；
   - pressure load；
   - 位移和 Von Mises 后处理。

### 建议新增/修改文件

- 新增 `samples/projects/single_tetra/`
- 新增 `samples/projects/box_pressure/`
- 新增 `src/validation/SampleProjectValidator.cpp`
- 新增 `src/validation/SampleProjectValidator.h`
- 新增 `src/ui/SampleValidationDialog.cpp`
- 新增 `src/ui/SampleValidationDialog.h`
- 修改 `src/ui/MainWindow.*`

### UI 建议

新增菜单：

```text
Tools -> Validate Samples
```

输出：

```text
[PASS] CalculiX executable found
[PASS] single_tetra solved
[PASS] box_pressure result read
[PASS] displacement field available
[PASS] Von Mises field available
```

### 验收标准

- 一键运行样例验证；
- 验证失败时能显示失败步骤；
- 验证结果写入 LogPanel 和 Diagnostics。

## 阶段 B3：项目资源管理

### 目标

统一管理项目里的几何、网格、求解结果和导出文件。

功能建议：

- 清理无效结果记录；
- 清理缺失文件的历史结果；
- 打开项目资源目录；
- 显示项目磁盘占用；
- 结果按时间排序；
- 可选择删除历史结果目录。

### 建议新增/修改文件

- 新增 `src/project/ProjectResourceManager.cpp`
- 新增 `src/project/ProjectResourceManager.h`
- 新增 `src/ui/ProjectResourceDialog.cpp`
- 新增 `src/ui/ProjectResourceDialog.h`
- 修改 `src/ui/MainWindow.*`

### 验收标准

- 可以列出 geometry/mesh/solver/results 文件；
- 可以识别无效 result；
- 可以只删除历史记录，也可以删除文件目录；
- 删除前必须二次确认。

## 阶段 B4：UI 状态持久化

### 目标

记住用户常用状态：

- 最近打开项目；
- 主窗口大小；
- dock 布局；
- 后处理面板字段；
- 后处理变形倍率；
- 最近导出目录。

### 建议新增/修改文件

- 新增 `src/ui/AppSettings.cpp`
- 新增 `src/ui/AppSettings.h`
- 修改 `src/ui/MainWindow.*`

### 技术建议

使用 Qt 原生：

```cpp
QSettings
```

### 验收标准

- 关闭再打开后，窗口布局恢复；
- 最近项目可从菜单打开；
- 后处理面板状态可以恢复。

## 阶段 B5：撤销/重做基础框架

### 目标

先建立可扩展的 undo/redo 框架，再逐步接入具体操作。

优先支持：

- 面组编辑；
- 材料编辑；
- 边界条件编辑；
- 载荷编辑；
- 结果显示设置修改。

### 建议新增/修改文件

- 新增 `src/commands/UndoStackController.cpp`
- 新增 `src/commands/UndoStackController.h`
- 修改现有 command 层；
- 修改 `src/ui/MainWindow.*`。

### 技术建议

优先考虑 Qt 原生：

```cpp
QUndoStack
QUndoCommand
```

### 验收标准

- 菜单有 `Edit -> Undo / Redo`；
- 至少一个面组操作可撤销；
- 撤销后树和渲染同步刷新。

## 推荐开发顺序

1. B1：错误诊断中心；
2. B2：样例工程自动验证；
3. B3：项目资源管理；
4. B4：UI 状态持久化；
5. B5：撤销/重做基础框架。

## 风险点

- 诊断中心不要替代日志，而是对日志做结构化摘要；
- 样例验证不要依赖用户桌面路径，应使用项目内 `samples/`；
- 删除资源必须区分“删除记录”和“删除磁盘文件”；
- UI 状态持久化不要影响首次启动；
- Undo/Redo 不要一次性接所有功能，先接一个闭环。

