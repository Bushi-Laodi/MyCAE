# MyCAE 用户使用说明

MyCAE 是一个轻量级 CAE 集成软件，当前主线支持：

- Qt 桌面 UI
- Open CASCADE 几何
- Gmsh 网格
- CalculiX 结构求解
- VTK 结果显示
- 位移、Von Mises、变形比例、色标范围、极值和点选查询

OpenFOAM 插件目前仍是预留状态，不参与实际求解。

## 1. 启动前检查

### 1.1 构建程序

常用构建方式：

```powershell
cmake --build .\build-codex-qt663-ninja --target MyCAE
```

或者在当前已配置的构建目录中直接运行：

```powershell
.\build-codex-qt663-ninja\MyCAE.exe
```

### 1.2 检查 Gmsh

菜单：

```text
工具 -> 检查 Gmsh
```

成功时日志应出现类似信息：

```text
Gmsh environment check succeeded.
```

### 1.3 检查 CalculiX

CalculiX 可执行文件优先级：

```text
1. 环境变量 MYCAE_CALCULIX_EXECUTABLE
2. CMake 定义 MYCAE_CALCULIX_EXECUTABLE_PATH
3. PATH 中的 ccx/ccx.exe
```

推荐在 PowerShell 中设置：

```powershell
$env:MYCAE_CALCULIX_EXECUTABLE="D:\path\to\ccx.exe"
```

如果路径不正确，CalculiX 插件可能仍能导出输入文件，但不能完成运行。

## 2. 新建工程

菜单：

```text
文件 -> 新建工程
```

选择一个空目录。成功后会生成：

```text
project.json
geometry/
mesh/
solver/
```

左侧 `工程 / 模型` 树会显示当前工程节点。

## 3. 创建几何

菜单：

```text
几何 -> 创建长方体
```

典型结构测试参数：

```text
长度 100 mm
宽度 20 mm
高度 20 mm
```

成功后：

- 项目树 `几何` 下出现 `Box_1`
- `geometry/` 下生成 `.json`、`.brep`、`.step`
- 中央视图显示几何预览

## 4. 创建面组

面组用于把几何面映射到边界条件、载荷和 Gmsh Physical Surface。

典型流程：

1. 选择 `Box_1`
2. 开启拾取面
3. 点击固定端面
4. 创建面组，例如 `FixedEnd`
5. 点击加载端面
6. 创建面组，例如 `LoadEnd`

如果面组创建在网格生成之前，Gmsh 会把它写入 Physical Surface，后续 CalculiX 边界映射更稳定。

## 5. 设置网格

选择几何后运行：

```text
仿真 -> 生成网格
```

成功后：

- `mesh/` 下生成 `.msh`
- 项目树 `网格` 下出现网格对象
- 日志中显示节点数、四面体数和 Physical Surface 信息

## 6. 设置仿真工况

### 6.1 材料

菜单：

```text
工况 -> 新建材料
```

常用钢材参数：

```text
E = 210000 MPa
nu = 0.3
```

### 6.2 边界条件

菜单：

```text
工况 -> 新建边界条件
```

选择固定端面组，例如 `FixedEnd`，设置位移约束。

### 6.3 载荷

菜单：

```text
工况 -> 新建载荷
```

选择加载端面组，例如 `LoadEnd`，设置压力或等效载荷。

工程会保存到：

```text
solver/case.json
```

## 7. 运行 CalculiX

菜单：

```text
仿真 -> 运行 CalculiX
```

执行流程：

```text
读取工程数据
-> 读取网格和面组
-> 构建 CalculiX case data
-> 导出 .inp
-> 调用 ccx
-> 读取 .sta/.dat/.frd/.log
-> 生成 ResultObject
```

成功后项目树 `结果` 下出现 CalculiX 结果记录。

## 8. 查看结果

选择结果节点后，右侧 `结果后处理` 面板可用。

当前支持：

- 结果场：`Ux`、`Uy`、`Uz`、`Displacement Magnitude`、`Von Mises Stress`
- 变形比例
- 显示/隐藏网格边
- 显示未变形轮廓
- 色标范围自动/锁定
- 节点和单元覆盖率
- 最小值/最大值
- 点选查询节点和单元结果
- CSV、报告、截图导出

## 9. 自动化验收

在构建目录运行：

```powershell
.\MyCAE.exe --validate-samples
.\MyCAE.exe --validate-ui
.\MyCAE.exe --capture-ui-sample --ui-screenshot-dir .\ui_screenshots
```

完整 CTest：

```powershell
ctest --test-dir .\build-codex-qt663-ninja --output-on-failure
```

预期：

```text
100% tests passed
```

## 10. 常见问题

### CalculiX 找不到

检查：

```powershell
$env:MYCAE_CALCULIX_EXECUTABLE
```

或检查 CMake 变量：

```text
MYCAE_CALCULIX_EXECUTABLE
```

### 网格没有边界映射

优先确认：

- 面组是否在生成网格前创建
- `.msh` 中是否有 Physical Surface
- 项目树中是否能看到面组和网格边界

### 结果能读但没有显示

检查：

- `.dat` 是否包含位移和应力
- 结果节点是否绑定了正确网格
- 右侧结果场是否选择了已有字段
- 日志面板是否有 result coverage 警告
