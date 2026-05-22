# MyCAE 求解前处理数据结构总览

> 本文档供开发小组成员参考，概述当前已设计完成的求解前处理数据结构。
> 对应源码位置：`src/solver/`

---

## 目录

1. [整体架构](#1-整体架构)
2. [阶段 8A：材料数据结构](#2-阶段-8a材料数据结构-materialh)
3. [阶段 8B：边界条件数据结构](#3-阶段-8b边界条件数据结构-boundaryconditionh)
4. [阶段 8C：载荷数据结构](#4-阶段-8c载荷数据结构-loadh)
5. [算例组合：SimulationCase](#5-算例组合-simulationcaseh)
6. [Pipe 算例示例](#6-pipe-算例示例-pipecaseexampleh)
7. [数据关系图](#7-数据关系图)
8. [下一步开发建议](#8-下一步开发建议)

---

## 1. 整体架构

```
SimulationCase（一个完整的算例）
├── GeometrySetup     几何设置（圆柱定义 + 布尔运算 + 面组）
├── MeshSetup         网格设置（尺寸、自动划分）
├── Material[]        材料列表（阶段 8A）
├── BoundaryCondition[]  边界条件列表（阶段 8B）
├── Load[]            载荷列表（阶段 8C）
├── FlowSolverType    求解器类型（SIMPLE）
├── TurbulenceModel   湍流模型（k-Omega SST）
└── RunControl        运行控制（结束时间、时间步长）
```

**核心设计思想**：边界条件和载荷分开。

| 概念 | 描述 | 示例 |
|---|---|---|
| **BoundaryCondition** | "什么类型的边界" | pipe.Inlet1 是速度入口 |
| **Load** | "具体数值是多少" | Inlet1 速度 = 0.8 m/s |

这样设计的好处：同一个边界条件类型（如速度入口）可以有不同的载荷值，方便参数化研究。

---

## 2. 阶段 8A：材料数据结构（Material.h）

**文件**：`src/solver/Material.h`

### 枚举

```cpp
enum class MaterialDomain { Fluid, Solid };

enum class ViscosityModel { Newtonian };
```

### 核心结构体

```cpp
struct MaterialProperty {
    QString name;       // 属性名称
    double  value;      // 属性值
    QString unit;       // 单位
};

struct Material {
    QString id;                     // 唯一标识，如 "fluid"
    QString name;                   // 显示名称，如 "Fluid"
    MaterialDomain domain;          // 流体 / 固体
    ViscosityModel viscosityModel;  // 粘度模型

    // 密度（可选，由 hasDensity 控制是否启用）
    bool    hasDensity;
    double  density;
    QString densityUnit;            // 默认 "kg/m^3"

    // 动力粘度（可选）
    bool    hasDynamicViscosity;
    double  dynamicViscosity;
    QString dynamicViscosityUnit;   // 默认 "Pa*s"

    // 运动粘度（可选）
    bool    hasKinematicViscosity;
    double  kinematicViscosity;
    QString kinematicViscosityUnit; // 默认 "m^2/s"

    // 额外属性（可扩展）
    std::vector<MaterialProperty> extraProperties;
};
```

### 设计说明

- 使用 `hasDensity` / `hasDynamicViscosity` / `hasKinematicViscosity` 布尔标志，表示该属性是否启用
- 这样在 UI 中可以根据标志决定是否显示输入框
- `extraProperties` 允许未来扩展任意材料属性（如热导率、比热容等）

---

## 3. 阶段 8B：边界条件数据结构（BoundaryCondition.h）

**文件**：`src/solver/BoundaryCondition.h`

### 枚举

```cpp
enum class BoundaryTargetKind {
    GeometryFaceGroup,  // 作用在几何面组上
    MeshBoundary        // 作用在网格边界上
};

enum class BoundaryConditionType {
    Wall,              // 壁面
    VelocityInlet,     // 速度入口
    PressureInlet,     // 压力入口
    PressureOutlet,    // 压力出口
    Symmetry,          // 对称面
    Unknown            // 未知
};
```

### 核心结构体

```cpp
struct BoundaryTarget {
    BoundaryTargetKind kind;           // 目标类型
    QString geometryName;              // 几何名称，如 "Pipe"
    QString faceGroupName;             // 面组名称，如 "Inlet1"
    QString meshBoundaryName;          // 网格边界名称（当 kind==MeshBoundary 时）
};

struct BoundaryCondition {
    QString id;                        // 唯一标识，如 "bc_inlet1"
    QString name;                      // 显示名称，如 "pipe.Inlet1"
    BoundaryConditionType type;        // 边界类型
    BoundaryTarget target;             // 作用目标
    QString materialId;                // 关联的材料 ID
    bool enabled;                      // 是否启用
};
```

### 设计说明

- `BoundaryTarget` 支持两种定位方式：几何面组（前处理阶段）和网格边界（网格划分后）
- `materialId` 关联到材料，因为不同边界可能属于不同材料区域
- `enabled` 允许临时禁用某个边界条件而不删除

---

## 4. 阶段 8C：载荷数据结构（Load.h）

**文件**：`src/solver/Load.h`

### 枚举

```cpp
enum class LoadType {
    Velocity,    // 速度
    Pressure,    // 压力
    BodyForce,   // 体积力
    Unknown
};

enum class LoadValueKind {
    Scalar,      // 标量值（如压力）
    Vector3      // 三维向量（如速度）
};
```

### 核心结构体

```cpp
struct LoadValue {
    LoadValueKind kind;   // 标量 / 向量
    double x;             // 标量值 或 向量 x 分量
    double y;             // 向量 y 分量
    double z;             // 向量 z 分量
    QString unit;         // 单位
};

struct Load {
    QString id;                        // 唯一标识，如 "load_inlet1_velocity"
    QString name;                      // 显示名称，如 "Inlet1 velocity"
    LoadType type;                     // 载荷类型
    QString boundaryConditionId;       // 关联的边界条件 ID
    QString fieldName;                 // 场名称，如 "U"（速度）、"p"（压力）
    LoadValue value;                   // 具体数值
    bool enabled;                      // 是否启用
};
```

### 设计说明

- `boundaryConditionId` 将载荷关联到边界条件，形成"边界条件类型 + 载荷数值"的配对
- `fieldName` 为后续求解器导出准备（OpenFOAM 中速度场叫 `U`，压力场叫 `p`）
- `LoadValue` 支持标量和向量两种形式，标量时只用 `x`，向量时用 `x/y/z`

---

## 5. 算例组合：SimulationCase（SimulationCase.h）

**文件**：`src/solver/SimulationCase.h`

### 枚举

```cpp
enum class FlowSolverType { Simple };           // SIMPLE 求解器
enum class TurbulenceModel { KOmegaSST };       // k-Omega SST 湍流模型
enum class AxisDirection { X, Y, Z };           // 轴向
enum class BooleanOperationType { Union };      // 布尔运算类型
```

### 几何相关结构体

```cpp
struct Point3D {
    double x, y, z;
};

struct CylinderDefinition {
    QString id;                  // 唯一标识
    QString name;                // 显示名称
    Point3D origin;              // 原点坐标
    AxisDirection direction;     // 轴向
    double length;               // 长度
    double radius;               // 半径
};

struct BooleanOperationDefinition {
    QString id;
    BooleanOperationType type;   // 布尔运算类型
    std::vector<QString> inputGeometryIds;  // 输入几何 ID 列表
    QString resultGeometryName;  // 结果几何名称
};

struct FaceGroupDefinition {
    QString name;                // 面组名称，如 "Inlet1"
    QString role;                // 角色，如 "inlet"
};
```

### 网格设置

```cpp
struct MeshSetup {
    double minimumSize;          // 最小网格尺寸
    double maximumSize;          // 最大网格尺寸
    bool   autoSize;             // 自动尺寸
    QString localFaceGroupName;  // 局部面组名称
    bool   autoImportAfterGeneration;   // 生成后自动导入
    bool   showBoundaryAfterImport;     // 导入后显示边界
};
```

### 运行控制

```cpp
struct RunControl {
    double endTime;              // 结束时间
    double timeStep;             // 时间步长
    double writeInterval;        // 写入间隔
    bool   cleanPreviousResult;  // 清除之前结果
};
```

### 算例主结构体

```cpp
struct SimulationCase {
    QString id;                              // 算例唯一标识
    QString name;                            // 算例名称
    QString sourceGeometryName;              // 源几何名称
    QString meshName;                        // 网格名称

    GeometrySetup geometrySetup;             // 几何设置
    MeshSetup meshSetup;                     // 网格设置

    FlowSolverType solverType;               // 求解器类型
    TurbulenceModel turbulenceModel;         // 湍流模型
    RunControl runControl;                   // 运行控制
    QString postProcessingTool;              // 后处理工具

    std::vector<Material> materials;         // 材料列表（阶段 8A）
    std::vector<BoundaryCondition> boundaryConditions;  // 边界条件列表（阶段 8B）
    std::vector<Load> loads;                 // 载荷列表（阶段 8C）
};
```

---

## 6. Pipe 算例示例（PipeCaseExample.h）

**文件**：`src/solver/PipeCaseExample.h`

函数 `createPipeSimulationCaseExample()` 创建了一个完整的 Pipe 算例，包含：

### 几何

| 对象 | 参数 |
|---|---|
| Cylinder1 | origin=(0,0,0), direction=Z, length=0.4, radius=0.05 |
| Cylinder2 | origin=(0,-0.4,0), direction=Y, length=1.5, radius=0.075 |
| Boolean Union | Cylinder1 ∪ Cylinder2 → "Pipe" |
| FaceGroups | Inlet1(inlet), Inlet2(inlet), Outlet(outlet) |

### 网格

| 参数 | 值 |
|---|---|
| minimumSize | 0.0 |
| maximumSize | 1.0 |
| autoSize | true |
| localFaceGroup | pipe.Default |

### 材料

| ID | 名称 | 域 | 粘度模型 |
|---|---|---|---|
| fluid | Fluid | Fluid | Newtonian |

### 边界条件

| 名称 | 类型 | 作用面 | 关联材料 |
|---|---|---|---|
| pipe.Default | Wall | Pipe.Default | fluid |
| pipe.Inlet1 | VelocityInlet | Pipe.Inlet1 | fluid |
| pipe.Inlet2 | PressureInlet | Pipe.Inlet2 | fluid |
| pipe.Outlet | PressureOutlet | Pipe.Outlet | fluid |

### 载荷

| 名称 | 类型 | 关联边界 | 数值 |
|---|---|---|---|
| Inlet1 velocity | Velocity | bc_inlet1 | 0.8 (标量) |
| Inlet2 pressure | Pressure | bc_inlet2 | 1.0 (标量) |

### 求解设置

| 参数 | 值 |
|---|---|
| Solver | SIMPLE |
| Turbulence | k-Omega SST |
| EndTime | 400 |
| CleanPrevious | false |
| PostProcessing | ParaView |

---

## 7. 数据关系图

```
SimulationCase
│
├── geometrySetup
│   ├── cylinders[] ────────── 定义基础几何
│   ├── booleanOperation ───── 布尔运算组合
│   └── faceGroups[] ───────── 定义面组（Inlet1, Inlet2, Outlet...）
│
├── meshSetup ──────────────── 网格划分参数
│
├── materials[] ────────────── 材料定义
│   └── id ─────────────────── 被 boundaryCondition.materialId 引用
│
├── boundaryConditions[]
│   ├── id ─────────────────── 被 load.boundaryConditionId 引用
│   ├── target.faceGroupName ─ 关联到 geometrySetup.faceGroups[].name
│   └── materialId ─────────── 关联到 materials[].id
│
├── loads[]
│   └── boundaryConditionId ── 关联到 boundaryConditions[].id
│
├── solverType ─────────────── 求解器算法
├── turbulenceModel ────────── 湍流模型
└── runControl ─────────────── 运行控制参数
```

### 关键关联

```
几何面组 (FaceGroupDefinition.name)
    ↑ 通过 target.faceGroupName 关联
边界条件 (BoundaryCondition)
    ↑ 通过 boundaryConditionId 关联
载荷 (Load)
```

---

## 8. 下一步开发建议

当前数据结构已设计完成，但尚未接入 UI。建议按以下顺序开发：

### 第一步：工程树交互（改动量小）

让工程树中已有的"材料"、"边界条件"、"载荷"节点可点击，属性面板显示对应内容。

### 第二步：创建/编辑对话框

参考 `BoxDialog` 的模式，创建：

- `MaterialDialog` — 创建/编辑材料
- `BoundaryConditionDialog` — 创建/编辑边界条件
- `LoadDialog` — 创建/编辑载荷

### 第三步：工程树子节点显示

在工程树中显示材料、边界条件、载荷的子节点，例如：

```
工程名
├── 几何
│   ├── Box_1
│   └── Box_2
├── 材料
│   └── Fluid
├── 边界条件
│   ├── pipe.Default (Wall)
│   ├── pipe.Inlet1 (VelocityInlet)
│   ├── pipe.Inlet2 (PressureInlet)
│   └── pipe.Outlet (PressureOutlet)
├── 载荷
│   ├── Inlet1 velocity (0.8)
│   └── Inlet2 pressure (1.0)
├── Mesh
└── 求解器
```

### 第四步：算例 JSON 序列化

实现 `SimulationCase` 的 JSON 保存和加载，与工程管理关联。

### 第五步：求解器导出

将 `SimulationCase` 导出为 CalculiX `.inp` 文件或 OpenFOAM 格式。
