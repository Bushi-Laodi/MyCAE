# MyCAE 求解器插件架构说明

本文档说明 MyCAE 当前的开放式求解器接口和插件化机制。目标是让主程序不绑定某一个固定求解器，而是通过统一接口对接 CalculiX、OpenFOAM、SU2、自研 Python/C++/Fortran 求解器等外部程序。

## 1. 设计目标

老师任务中的关键要求是：

```text
设计标准化的求解器接口和插件化机制，让求解器集成变得简单。
```

MyCAE 当前采用的思路是：

```text
SimulationCase
  -> SolverPlugin 统一接口
  -> ExternalProcessSolverPlugin
  -> 外部求解器脚本或可执行程序
  -> result.json
```

这样主程序只依赖 `SolverPlugin`，不直接依赖某个具体求解器。

## 2. 当前代码结构

```text
src/solver/
  SimulationCase.h
  SimulationCaseJsonWriter.h
  SimulationCaseJsonWriter.cpp

src/solver/plugin/
  SolverPlugin.h
  SolverPluginManager.h
  SolverPluginManager.cpp
  ExternalProcessSolverPlugin.h
  ExternalProcessSolverPlugin.cpp
  ExternalSolverPluginConfig.h
  ExternalSolverPluginConfig.cpp

resource/solver/demo_solver/
  solver_plugin.json
  demo_solver.py
```

各文件职责：

```text
SolverPlugin
  标准求解器接口。

SolverPluginManager
  扫描 resource/solver/*/solver_plugin.json，自动注册求解器插件。

ExternalProcessSolverPlugin
  通用外部进程插件实现，负责导出输入、调用外部程序、读取结果。

ExternalSolverPluginConfig
  读取 solver_plugin.json。

SimulationCaseJsonWriter
  将 SimulationCase 导出为 case.json。
```

## 3. 标准接口

所有求解器插件都遵守 `SolverPlugin` 接口：

```cpp
class SolverPlugin
{
public:
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    virtual bool exportCase(
        const SimulationCase &simulationCase,
        const QString &caseDirectory,
        QString *errorMessage
    ) const = 0;

    virtual bool runCase(
        const QString &caseDirectory,
        QString *logText,
        QString *errorMessage
    ) const = 0;

    virtual bool readResult(
        const QString &caseDirectory,
        QString *resultText,
        QString *errorMessage
    ) const = 0;
};
```

三个核心动作：

```text
exportCase
  将 MyCAE 内部算例数据导出为求解器输入文件。

runCase
  调用外部求解器。

readResult
  读取求解器结果并返回给 MyCAE。
```

## 4. 插件目录规范

每个外部求解器使用一个独立目录：

```text
resource/solver/{solver_id}/
  solver_plugin.json
  solver_script.py 或 solver.exe
```

当前 Demo 插件：

```text
resource/solver/demo_solver/
  solver_plugin.json
  demo_solver.py
```

`SolverPluginManager` 会自动扫描：

```text
resource/solver/*/solver_plugin.json
```

只要目录中存在 `solver_plugin.json`，就会注册为一个外部求解器插件。

## 5. solver_plugin.json 字段

示例：

```json
{
    "id": "demo",
    "name": "External Demo Solver",
    "type": "python",
    "script": "demo_solver.py",
    "inputFile": "case.json",
    "outputFile": "result.json",
    "supportedAnalysisTypes": [
        "demo",
        "linearStatic",
        "cfd"
    ],
    "description": "Minimal external solver wrapper demo for validating the MyCAE solver plugin interface."
}
```

字段说明：

```text
id
  插件唯一标识，用于 pluginById(id) 查找。

name
  插件显示名称，会出现在 Simulation 菜单中。

type
  外部程序类型。当前支持 python。

executable
  可选。显式指定外部程序，例如 solver.exe 或 python.exe。

script
  外部脚本文件名。相对于插件目录。

inputFile
  MyCAE 导出的输入文件名，当前为 case.json。

outputFile
  外部求解器输出文件名，当前为 result.json。

supportedAnalysisTypes
  插件声明支持的分析类型，供后续 UI 过滤和校验使用。

description
  插件说明。
```

## 6. 输入输出约定

当前通用外部求解器插件采用两个文件作为最小协议：

```text
case.json
  MyCAE 导出的算例输入。

result.json
  外部求解器输出的结果摘要。
```

调用流程：

```text
MyCAE
  -> exportCase()
  -> 写出 case.json
  -> runCase()
  -> 调用外部程序
  -> 外部程序读取 case.json
  -> 外部程序写出 result.json
  -> readResult()
  -> MyCAE 显示结果摘要
```

Demo 脚本命令形式：

```text
python demo_solver.py case.json result.json
```

当前 `result.json` 示例：

```json
{
    "solverId": "demo",
    "solverName": "External Demo Solver",
    "status": "completed",
    "caseId": "test_case",
    "caseName": "Test Case",
    "iterations": 1,
    "maxResidual": 0.0
}
```

## 7. UI 接入方式

`MainWindow` 不再写死某个求解器菜单，而是遍历插件管理器：

```text
m_solverPluginManager.plugins()
```

然后自动生成：

```text
Simulation
  Run External Demo Solver
```

后续新增 CalculiX、OpenFOAM、SU2 插件后，也会自动出现在 `Simulation` 菜单中。

## 8. 如何接入一个自研求解器

以 Python 求解器为例：

1. 新建目录：

```text
resource/solver/my_solver/
```

2. 放入描述文件：

```text
resource/solver/my_solver/solver_plugin.json
```

3. 放入求解器脚本：

```text
resource/solver/my_solver/my_solver.py
```

4. 脚本遵守命令行约定：

```text
python my_solver.py case.json result.json
```

5. 脚本读取 `case.json`，写出 `result.json`。

6. 重新构建或复制资源后，MyCAE 会自动发现该插件。

对于 C++ 或 Fortran 求解器，可以将编译好的可执行程序放入插件目录，并在 `solver_plugin.json` 中配置 `executable`。

## 9. 与 CalculiX / OpenFOAM 的关系

当前 Demo 插件用于验证开放架构，不承担真实物理求解。

后续接 CalculiX 时，可以新增：

```text
resource/solver/calculix_solver/
  solver_plugin.json
  ccx_runner.exe 或 ccx_runner.py
```

其内部流程可以是：

```text
case.json
  -> 生成 CalculiX .inp
  -> 调用 ccx
  -> 读取 .dat / .frd
  -> 生成 result.json
```

后续接 OpenFOAM 时，可以新增：

```text
resource/solver/openfoam_solver/
  solver_plugin.json
  openfoam_runner.py
```

其内部流程可以是：

```text
case.json
  -> 生成 OpenFOAM case
  -> 写 system / constant / 0
  -> 调用 blockMesh / simpleFoam
  -> 读取 residuals / postProcessing
  -> 生成 result.json
```

主程序仍然只调用：

```text
exportCase()
runCase()
readResult()
```

## 10. 当前限制

当前插件机制还是最小版本，有以下限制：

```text
1. 外部求解器输入协议暂时固定为 case.json。
2. 结果读取暂时只读取 result.json 摘要。
3. 尚未实现完整场数据、云图、时间步结果导入。
4. 尚未做插件选择面板和参数编辑界面。
5. 尚未做真实 CalculiX / OpenFOAM 文件导出。
```

这些限制不影响当前阶段目标。当前阶段的重点是证明：

```text
MyCAE 已经具备开放式求解器接入机制。
新增求解器不需要改 MainWindow。
外部求解器只要遵守输入输出约定即可接入。
```
