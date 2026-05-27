# MyCAE 文档索引

本文档目录面向两类读者：

- 使用者：想打开样例、建模、划分网格、运行 CalculiX、查看结果。
- 开发者：想理解 MyCAE 的模块边界、数据流、求解器插件和 UI 架构。

## 文档列表

| 文档 | 用途 |
| --- | --- |
| [用户使用说明](user-guide.md) | 从启动软件到完成一次结构仿真的操作流程。 |
| [样例工程说明](sample-projects.md) | 说明内置样例工程和 CalculiX 输入 deck 的内容、目录结构和验收命令。 |
| [求解流程说明](solver-workflow.md) | 解释几何、面组、网格、工况、求解器、结果读取之间的数据流。 |
| [开发架构说明](development-architecture.md) | 说明代码模块职责、关键类、控制器拆分和后续扩展方向。 |

## 快速验收

在已经构建好的目录中运行：

```powershell
.\MyCAE.exe --validate-samples
.\MyCAE.exe --validate-ui
.\MyCAE.exe --capture-ui-sample --ui-screenshot-dir .\ui_screenshots
```

如果使用 CTest：

```powershell
ctest --test-dir .\build-codex-qt663-ninja --output-on-failure
```

预期结果：

```text
100% tests passed
```
