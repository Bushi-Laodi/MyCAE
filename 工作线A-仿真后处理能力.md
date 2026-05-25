# 工作线 A：仿真后处理能力

## 目标

把 MyCAE 的 CalculiX 后处理从“能看云图”推进到“能分析、能导出、能汇报”。

当前基础已经具备：

- CalculiX 可以导出、运行、读取结果；
- 结果树可以显示历史结果；
- VTK 可以显示位移云图、Von Mises 应力云图、变形图；
- 右侧已有 Result Postprocess 面板；
- 结果可持久化到 `solver/results.json`。

本工作线继续增强结果分析能力。

## 模块边界

主要涉及：

- `src/result/`
- `src/solver/calculix/`
- `src/ui/ResultPostprocessPanel.*`
- `src/ui/RenderView.*`
- `src/ui/VtkRenderCanvas.*`

尽量不修改：

- 几何建模模块；
- Gmsh 网格生成主流程；
- 材料/边界/载荷编辑流程；
- OpenFOAM 预留插件。

## 阶段 A1：结果极值定位

### 目标

找出并显示：

- 最大位移节点；
- 最大 `Ux / Uy / Uz / |U|` 节点；
- 最大 Von Mises 应力单元；
- 对应 ID、坐标、数值；
- 在 VTK 视图中高亮极值位置。

### 建议新增/修改文件

- 新增 `src/result/ResultExtrema.h`
- 新增 `src/result/ResultExtremaCalculator.cpp`
- 新增 `src/result/ResultExtremaCalculator.h`
- 修改 `src/result/ResultObject.h`
- 修改 `src/result/ResultDisplayController.cpp`
- 修改 `src/ui/ResultPostprocessPanel.*`
- 修改 `src/ui/VtkRenderCanvas.*`

### 数据结构建议

```cpp
struct ResultNodeExtreme
{
    int nodeId = 0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double value = 0.0;
    QString fieldName;
};

struct ResultElementExtreme
{
    int elementId = 0;
    double value = 0.0;
    QString fieldName;
};
```

### 验收标准

- 选择 CalculiX Result 后，面板能显示最大值；
- 切换字段后，极值同步变化；
- 最大值位置能在 VTK 里高亮；
- 日志输出类似：

```text
Result extrema: field=Displacement Magnitude, maxNode=57, value=...
Result extrema: field=Von Mises Stress, maxElement=312, value=...
```

## 阶段 A2：CSV 数据导出

### 目标

导出两类结果表：

节点位移：

```text
nodeId,x,y,z,Ux,Uy,Uz,UMag
```

单元应力：

```text
elementId,VonMises
```

### 建议新增/修改文件

- 新增 `src/result/ResultCsvExporter.cpp`
- 新增 `src/result/ResultCsvExporter.h`
- 修改 `src/ui/ResultPostprocessPanel.*`
- 修改 `src/ui/MainWindow.*`

### UI 建议

在 `Result Postprocess` 面板增加：

- `Export CSV` 按钮；
- 导出路径默认：

```text
<project>/solver/exports/<result-name>_result.csv
```

### 验收标准

- 能导出节点位移 CSV；
- 能导出单元 Von Mises CSV；
- CSV 可用 Excel / Python 正常读取；
- 如果结果缺失，弹窗或日志明确提示。

## 阶段 A3：结果报告导出

### 目标

导出一个 Markdown 或 HTML 报告，包含：

- 项目名；
- solver 名称；
- case 路径；
- mesh 名称；
- 当前显示字段；
- scalar min/max；
- 最大位移；
- 最大 Von Mises；
- 结果完整性检查；
- 当前截图路径。

### 建议新增/修改文件

- 新增 `src/result/ResultReportExporter.cpp`
- 新增 `src/result/ResultReportExporter.h`
- 修改 `src/ui/ResultPostprocessPanel.*`
- 修改 `src/ui/MainWindow.*`

### 验收标准

- 点击 `Export Report` 能生成 `.md`；
- 报告里能看到截图引用；
- 报告内容可直接用于课程项目或论文阶段记录。

## 阶段 A4：变形动画

### 目标

播放变形动画：

```text
0 -> 当前 deformationScale -> 0
```

### 建议新增/修改文件

- 新增 `src/result/ResultAnimationController.cpp`
- 新增 `src/result/ResultAnimationController.h`
- 修改 `src/ui/ResultPostprocessPanel.*`
- 修改 `src/ui/MainWindow.*`

### UI 建议

在后处理面板增加：

- `Play`
- `Stop`
- speed 输入或滑块；
- 当前帧倍率显示。

### 验收标准

- 播放时云图网格随倍率变化；
- Stop 后停在当前倍率；
- 不影响字段切换和截图导出。

## 推荐开发顺序

1. A1：结果极值定位；
2. A2：CSV 数据导出；
3. A3：报告导出；
4. A4：变形动画。

## 风险点

- 应力目前来自 `.dat` 单元积分点，需要继续确认不同 CalculiX 输出格式；
- 极值高亮需要区分 point scalar 和 cell scalar；
- CSV/报告导出要避免把绝对路径写死到不可迁移位置；
- 动画不要阻塞 Qt UI 线程，建议用 `QTimer`。

