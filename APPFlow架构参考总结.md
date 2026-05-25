# APPFlow 架构参考总结

本文档用于给 MyCAE 项目组同事参考，重点总结 APPFlow 的架构特点，以及 MyCAE 可以学习但不必照搬的设计。

## 1. 总体判断

APPFlow 不是简单的 Qt 示例程序，而是一套基于 FastCAE/FITK 框架的成熟前处理应用。它的整体设计比 MyCAE 当前架构更工程化，尤其在用户动作分发、树/属性/渲染联动、图形拾取、全局数据管理等方面值得参考。

MyCAE 当前架构更轻、更直接，适合快速推进原型。APPFlow 的价值在于提供后续扩展方向：当功能越来越多时，需要避免 `MainWindow` 和中心数据模型变成所有逻辑的堆积点。

## 2. APPFlow 顶层模块划分

APPFlow 目录结构大致如下：

```text
APPFlow
├─ FlowApp              应用入口、全局工厂、工作台处理
├─ GUIFrame             主窗口、Ribbon、树、属性面板、渲染面板
├─ GUIWidget            通用 UI 控件、树控件、拾取数据管理
├─ GUIDialog            各类参数编辑窗口
├─ OperatorsInterface   操作器接口：树事件、图形事件、参数窗口事件
├─ OperatorsModel       业务操作器：几何、网格、求解设置、运行、保存
├─ OperatorsGUI         图形交互操作器：视图、拾取、渲染更新
├─ GraphDataProvider    图形数据提供相关
├─ FITK_*               框架、组件、接口子模块
└─ Resources            图标和资源
```

它的核心思想是：主窗口只负责组织界面，具体业务动作由 Operator 体系承接。

## 3. 核心设计一：Action 到 Operator 的解耦

MyCAE 当前很多 QAction 是直接连接到 `MainWindow` 的成员函数，例如创建几何、生成网格、运行求解器等。

APPFlow 的做法不同。它通过 QAction 的 `objectName` 找到对应 Operator：

```text
QAction.objectName
  -> ActionEventHandler
  -> FITKOperatorRepo
  -> 对应 Operator
  -> 执行业务逻辑
```

例如：

```text
actionGeoCubeCreate        -> OperatorsGeoCubeManager
actionGeoCylinderCreate    -> OperatorsGeoCylinderManager
actionMesh                 -> OperatorsMeshManager
actionRun                  -> OperatorsRun
actionSave                 -> OperatorsSave
```

这个设计的好处是：

- `MainWindow` 不需要知道每个按钮背后的业务细节。
- 新增功能时主要新增 Operator，而不是继续修改主窗口。
- 菜单、工具栏、右键菜单、树节点点击都可以复用同一套 action/operator。

对 MyCAE 的启发：可以实现一个轻量版 `ActionRegistry` 或 `CommandRegistry`，不必照搬 FITK 的完整 OperatorRepo。

## 4. 核心设计二：操作拆成 GUI 阶段和业务阶段

APPFlow 的业务 Operator 常见结构是：

```cpp
bool execGUI();
bool execProfession();
```

含义大致是：

```text
execGUI         负责打开参数面板、对话框、准备用户输入
execProfession  负责真正修改数据、执行计算、刷新树和渲染
```

例如创建几何时：

```text
execGUI
  -> 在属性面板显示几何参数编辑 Widget

execProfession
  -> 创建或修改几何数据
  -> 更新模型树
  -> 更新三维显示
  -> 重新渲染
```

MyCAE 现在已经有一些类似趋势，例如：

```text
GeometryCreationController
MeshWorkflowController
SolverDataController
```

但还没有统一模式。后续可以考虑把主要用户行为统一抽象成：

```text
Command::prepareUi()
Command::execute()
Command::refreshUi()
```

这样能让创建几何、生成网格、创建材料、编辑边界条件、运行求解器等流程具有一致结构。

## 5. 核心设计三：树、属性面板、渲染统一联动

APPFlow 的树节点点击后，并不是树控件直接完成所有逻辑，而是根据节点类型找到对应 action/operator。

典型流程：

```text
用户点击树节点
  -> 根据节点类型得到 actionName
  -> 从 OperatorRepo 找到 Operator
  -> 设置 objID 等参数
  -> 执行 Operator
  -> 切换属性面板
  -> 高亮或刷新三维图形
```

这种方式让树控件不需要直接了解所有业务细节。

MyCAE 当前 `ProjectTreePanel` 通过多个 signal 分发选择事件：

```text
geometrySelected
meshSelected
faceGroupSelected
materialSelected
boundaryConditionSelected
loadSelected
```

短期清楚，长期随着对象类型增多会变散。可以考虑引入统一选择事件：

```cpp
enum class SelectionKind {
    Geometry,
    Mesh,
    FaceGroup,
    Material,
    BoundaryCondition,
    Load
};

struct Selection {
    SelectionKind kind;
    QString id;
    QString name;
};
```

然后由 `SelectionController` 决定：

```text
显示哪个属性面板
是否刷新渲染
是否高亮对象
是否启用某些 action
```

## 6. 核心设计四：图形拾取系统

APPFlow 有比较完整的图形拾取设计，主要包括：

```text
GUIPickInfo
  当前拾取模式：点、边、面、体；单选、多选等

PickedDataProvider
  保存拾取结果、预选结果、生成高亮数据

GraphEventOperator
  统一处理图形更新、显示隐藏、高亮、刷新
```

它支持的能力包括：

- 点、边、面、体不同拾取类型。
- 单选、多选。
- 预选高亮。
- Shift 增量选择。
- Ctrl 删除选择。
- 拾取结果转换为可视化高亮数据。

这对 MyCAE 后续非常重要。因为 CAE 前处理迟早需要：

```text
选择入口面
选择出口面
选择壁面
建立 FaceGroup
给面组绑定边界条件
给局部区域设置网格加密
```

MyCAE 当前已有 `FaceGroup` 概念，但还缺少完整的交互拾取系统。APPFlow 的拾取架构可以作为后续设计参考。

## 7. 核心设计五：全局数据分区

APPFlow 通过 `GlobalDataFactory` 创建不同类型的数据区：

```text
GeometryData
MeshData
PhysicsData
PostData
```

相比之下，MyCAE 当前的 `ProjectModel` 同时保存：

```text
Project
GeometryObject[]
BoxGeometry[]
CylinderGeometry[]
MeshObject[]
Material[]
BoundaryCondition[]
Load[]
FaceGroup[]
当前选中项
```

当前阶段这样做开发效率高，但随着功能变多，`ProjectModel` 会越来越大，而且会混合项目数据、业务数据、UI 状态。

建议 MyCAE 后续逐步演进为：

```text
ApplicationModel
├─ ProjectData
├─ GeometryDataStore
├─ MeshDataStore
├─ SolverDataStore
├─ RenderState
└─ SelectionState
```

尤其建议优先把“当前选中项”从项目数据里拆出去，形成独立的 `SelectionState`。

## 8. MyCAE 不建议照搬的部分

APPFlow 的架构成熟，但复杂度也明显更高。MyCAE 不建议直接复制以下设计：

- 不建议现在就引入完整 FITK 式框架。
- 不建议大量使用全局单例访问所有数据。
- 不建议过早把每个小动作都做成复杂 Operator。
- 不建议过度依赖字符串 actionName 做业务分发，容易出现运行时才发现的命名错误。
- 不建议把 APPFlow 的 Ribbon UI 作为当前优先目标，MyCAE 当前更应该优先稳定数据和流程架构。

MyCAE 当前更适合做“轻量版 APPFlow 思路”。

## 9. 对 MyCAE 的具体改造建议

## 9.0 建议 MyCAE 借鉴的优先级

第一优先级：拆 `MainWindow`

引入轻量 `ActionCommand` 或 `WorkflowController` 注册机制，让 `MainWindow` 只负责 UI 组装和事件入口。也就是说，按钮、菜单、树节点点击仍然可以从 `MainWindow` 进入，但具体业务逻辑不要继续堆在 `MainWindow` 里。

第二优先级：引入统一选择模型

用 `SelectionState` 替代多个分散的选择字段，例如：

```text
selectedGeometryName
selectedMeshName
selectedMaterialId
selectedBoundaryConditionId
selectedLoadId
```

统一选择模型可以让树选择、属性面板刷新、渲染高亮、可用 action 状态都走同一套逻辑。

第三优先级：设计拾取/面组系统

参考 APPFlow 的：

```text
GUIPickInfo
PickedDataProvider
GraphEventOperator
```

为后续边界条件绑定面组做准备。MyCAE 后续如果要支持入口、出口、壁面、局部网格加密等功能，必须能在三维视图里稳定选择面，并把拾取结果保存成 `FaceGroup`。

第四优先级：拆分 `ProjectModel`

把几何、网格、求解器数据分成独立 store，避免中心模型越来越大。建议后续逐步演进为：

```text
GeometryStore
MeshStore
SolverDataStore
SelectionState
RenderState
```

这样 `ProjectModel` 或 `ProjectContext` 只负责组合这些模块，而不是直接承载所有数据和 UI 状态。

### 9.1 第一优先级：拆 MainWindow

当前 MyCAE 的 `MainWindow` 同时负责：

```text
菜单和工具栏
项目创建/打开
树刷新
几何创建
几何显示
网格生成
网格显示
求解器数据管理
求解器运行
日志输出
```

建议逐步拆出：

```text
ProjectWorkflowController
GeometryWorkflowController
MeshWorkflowController
SolverWorkflowController
SelectionController
```

其中 `MeshWorkflowController` 已经存在，可以继续沿这个方向扩展。

### 9.2 第二优先级：引入轻量 Command/ActionRegistry

可以设计成：

```cpp
class AppCommand {
public:
    virtual ~AppCommand() = default;
    virtual void execute() = 0;
};

class ActionRegistry {
public:
    void registerCommand(const QString &actionId, std::unique_ptr<AppCommand> command);
    void execute(const QString &actionId);
};
```

QAction 只负责提供 `actionId`，具体逻辑由 command 执行。

这样可以逐步替代：

```cpp
connect(action, &QAction::triggered, this, &MainWindow::someBusinessFunction);
```

### 9.3 第三优先级：统一选择模型

建议新增：

```text
SelectionState
SelectionController
```

用于统一处理：

```text
树节点选择
属性面板刷新
渲染高亮
当前可用操作
```

这比在 `ProjectModel` 里继续增加 `selectedXXX` 更可控。

### 9.4 第四优先级：设计拾取和面组系统

建议参考 APPFlow 的思路，设计 MyCAE 自己的轻量拾取系统：

```text
PickMode
  当前拾取点/边/面/体

PickSelection
  当前拾取结果

PickController
  负责拾取逻辑和选择模式

RenderHighlightController
  负责高亮显示
```

这对后续做边界条件、局部网格、面组管理非常关键。

### 9.5 第五优先级：拆分 ProjectModel

后续可以逐步从：

```text
ProjectModel
```

演进到：

```text
ProjectContext
├─ Project
├─ GeometryRepository
├─ MeshRepository
├─ SolverRepository
└─ SelectionState
```

这样各模块之间边界会更清楚。

## 10. 推荐演进路线

建议 MyCAE 不要一次性大改，而是按下面顺序渐进调整：

```text
阶段 1：保留现有架构，先拆 MainWindow 中最重的业务函数
阶段 2：引入 SelectionState，统一树选择和属性面板刷新
阶段 3：引入轻量 ActionRegistry/Command
阶段 4：设计拾取系统和 FaceGroup 交互流程
阶段 5：拆分 ProjectModel 为多个数据仓库
阶段 6：稳定求解器插件 schema 和运行流程
```

## 11. 总结

APPFlow 对 MyCAE 最有价值的参考不是某个具体类，而是它的整体架构思想：

```text
UI 动作不要直接写业务；
树、属性面板、渲染不要互相硬连；
用户行为通过统一操作器进入；
操作器负责协调数据、属性面板、树刷新和图形刷新；
几何、网格、物理场、后处理数据应逐步分区管理。
```

MyCAE 当前不需要推倒重来。更合适的路线是保留现在清晰直接的结构，同时吸收 APPFlow 的轻量版思想，优先解决 `MainWindow` 膨胀、选择状态分散、图形拾取缺失、`ProjectModel` 过重这几个问题。
