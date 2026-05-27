# MyCAE

MyCAE 是一个基于 Qt 6 的轻量级 CAE 集成项目，当前主线已经覆盖：

- Open CASCADE 几何
- Gmsh 网格
- CalculiX 结构求解
- VTK 结果显示
- 结果后处理、极值、点选查询和导出
- 样例工程与自动化验收

## 文档

统一文档入口：

[docs/README.md](docs/README.md)

主要文档：

- [用户使用说明](docs/user-guide.md)
- [样例工程说明](docs/sample-projects.md)
- [求解流程说明](docs/solver-workflow.md)
- [开发架构说明](docs/development-architecture.md)

## 快速构建

```powershell
cmake --build .\build-codex-qt663-ninja --target MyCAE
```

## 快速验收

```powershell
.\build-codex-qt663-ninja\MyCAE.exe --validate-samples
.\build-codex-qt663-ninja\MyCAE.exe --validate-ui
.\build-codex-qt663-ninja\MyCAE.exe --capture-ui-sample --ui-screenshot-dir .\build-codex-qt663-ninja\ui_screenshots
```

或使用 CTest：

```powershell
ctest --test-dir .\build-codex-qt663-ninja --output-on-failure
```

预期结果：

```text
100% tests passed
```
