# MyCAE 对话式开发记录

这份文档按我们搭建项目时的对话思路整理，重点回答三个问题：

```text
为什么先做这一步？
这一步实际改了什么？
下一步应该怎么继续？
```

它不是完整源码说明书，而是给后续学习和继续开发用的路线图。

## 0. 当前项目已经做到什么程度？

你现在已经有了一个基于 Qt6 + VTK 的轻量级 CAE 原型。

当前可运行闭环是：

```text
启动 MyCAE
  -> 新建 / 打开工程
  -> 创建 Box 几何对象
  -> 保存 geometry/box_*.json
  -> 工程树显示 Box_1 / Box_2
  -> 属性面板显示长宽高
  -> VTK 中央视图显示立方体
  -> 日志窗口输出操作过程
```

当前还没有做：

```text
Open CASCADE 真实几何
Gmsh 网格划分
CalculiX 求解
后处理云图
```

所以现在的软件定位是：

```text
CAE 工作流原型
  不是完整 CAE 软件
  不是最终 CAD/CAE 内核
  是后续接 OCC / VTK / Gmsh / CalculiX 的基础骨架
```

## 1. 一开始为什么不直接写完整大工程？

你一开始说：“现在我要开始搭建这个项目了。”

我的建议是先做最小可运行版本，而不是一次性生成完整工程。

原因是这个项目涉及：

```text
Qt
Open CASCADE
VTK
Gmsh
CalculiX
工程管理
几何数据
网格数据
求解数据
后处理数据
```

如果一开始全接上，任何一个地方出错都很难判断问题来源。

所以我们采用路线：

```text
项目骨架
  -> 工程管理
  -> 最小几何数据
  -> 属性和工程树联动
  -> 可视化显示
  -> 再接第三方几何/网格/求解库
```

这条路线的核心原则是：

```text
每一步都能运行
每一步都能验证
每一步只引入一个主要复杂点
```

## 2. 为什么先做 Qt6 主界面？

你的要求是用 Qt6 编写。

所以第一步做了一个 Qt6 Widgets 主界面。

当前主界面包含：

```text
菜单栏
工具栏
左侧工程树
右侧属性面板
底部日志窗口
中央显示区域
```

这些区域对应一个 CAE 软件最基本的工作区：

```text
工程树
  管理模型、材料、边界条件、网格、求解任务

属性面板
  显示和编辑当前对象参数

中央视图
  显示几何、网格、结果

日志窗口
  显示外部工具和内部流程信息
```

对应源码：

```text
src/ui/MainWindow.h
src/ui/MainWindow.cpp
```

## 3. 为什么要把 UI 拆成多个文件？

你问：“是不是把界面的各个部分分成不同的文件会好一点，然后再统一组装？”

答案是：是的。

如果所有 UI 都写在 `MainWindow.cpp`，后面会变成：

```text
MainWindow
  管菜单
  管工具栏
  管工程树
  管属性面板
  管日志
  管渲染
  管工程数据
  管几何创建
```

这会让代码越来越难改。

所以我们把 UI 拆成：

```text
MainWindow
  总装配和总协调

ProjectTreePanel
  左侧工程树

PropertyPanel
  右侧属性面板

LogPanel
  底部日志

RenderView
  中央显示区域外壳
```

对应源码：

```text
src/ui/MainWindow.h
src/ui/MainWindow.cpp
src/ui/ProjectTreePanel.h
src/ui/ProjectTreePanel.cpp
src/ui/PropertyPanel.h
src/ui/PropertyPanel.cpp
src/ui/LogPanel.h
src/ui/LogPanel.cpp
src/ui/RenderView.h
src/ui/RenderView.cpp
```

这一步没有改变功能，只改变结构。

## 4. 为什么第二步做工程管理？

你后面要做的是 CAE 软件，不只是一个显示窗口。

CAE 软件的核心不是先画图，而是先有工程目录管理：

```text
project.json
geometry/
mesh/
solver/
logs/
```

所以我们实现了：

```text
File -> New Project
  -> 选择目录
  -> 创建 project.json
  -> 创建 geometry / mesh / solver / logs
  -> 工程树刷新
  -> 日志输出

File -> Open Project
  -> 选择 project.json
  -> 读取工程信息
  -> 工程树刷新
```

对应源码：

```text
src/project/Project.h
src/project/ProjectManager.h
src/project/ProjectManager.cpp
```

`Project` 只保存工程基本信息：

```text
工程名
工程根目录
project.json 路径
```

`ProjectManager` 负责：

```text
创建工程目录
写入 project.json
读取已有工程
防止覆盖已有 project.json
```

## 5. 为什么先做假的 Box，而不是直接接 Open CASCADE？

你问过：“后面接 OCC 的时候，这些 geometry 还有用的必要吗？”

结论是：

```text
geometry 模块有必要保留
但现在的 BoxGeometry 不是最终几何内核
```

现在的 `BoxGeometry` 是学习型过渡结构，用来先跑通数据流：

```text
用户点击 Create Box
  -> 创建 BoxGeometry
  -> 保存 geometry/box_1.json
  -> 工程树显示 Box_1
  -> 属性面板显示尺寸
```

它让我们先验证：

```text
用户操作
数据对象
JSON 保存
工程树刷新
属性面板显示
```

这些流程以后接 OCC 仍然有用。

后面接 OCC 后会变成：

```text
现在：
BoxGeometry
  -> 保存 length / width / height / unit
  -> VTK 用 vtkCubeSource 显示

以后：
GeometryObject
  -> 保存几何对象元数据

OCC:
  -> 根据参数创建 TopoDS_Shape
  -> 保存 BREP / STEP
  -> 转成 vtkPolyData 显示
```

对应源码：

```text
src/geometry/BoxGeometry.h
src/geometry/GeometryManager.h
src/geometry/GeometryManager.cpp
```

当前 Box JSON 示例：

```json
{
    "createdAt": "2026-05-15T16:00:00",
    "dimensions": {
        "height": 80,
        "length": 300,
        "unit": "mm",
        "width": 120
    },
    "name": "Box_1",
    "type": "box"
}
```

## 6. 为什么加 Box 参数输入对话框？

一开始 `Create Box` 固定生成：

```text
200 mm x 200 mm x 200 mm
```

这只能证明流程跑通，但不像一个真正的 CAE 前处理软件。

所以我们加了：

```text
BoxDialog
```

用于输入：

```text
length
width
height
unit
```

对应源码：

```text
src/ui/BoxDialog.h
src/ui/BoxDialog.cpp
```

它的职责很单纯：

```text
只负责收集用户输入
不保存文件
不刷新工程树
不直接操作 VTK
```

保存文件仍然由：

```text
GeometryManager
```

负责。

这个模式后面可以复用到：

```text
材料参数对话框
边界条件对话框
载荷对话框
网格参数对话框
求解器设置对话框
```

## 7. 为什么先做 2D 占位显示，再接 VTK？

一开始中央显示区只是文字占位。

我们先让它参与数据流：

```text
点击 Box_1
  -> 属性面板显示参数
  -> 中央视图显示 Box 名称和尺寸
```

然后又做了一个简单 2D 线框预览：

```text
RenderCanvas
  -> Qt paintEvent()
  -> 按 length / width / height 绘制线框
```

对应源码：

```text
src/ui/RenderCanvas.h
src/ui/RenderCanvas.cpp
```

这一步的目的不是做最终显示，而是先建立：

```text
几何数据
  -> 视图组件
  -> 预览显示
```

这一步现在保留为回退和参考。

## 8. 为什么后面又拆出 RenderView 和 RenderCanvas？

在接 VTK 前，我们先做了一个视图抽象。

原因是后面 VTK 显示区域通常不是普通 QWidget 绘制，而是：

```text
QVTKOpenGLNativeWidget
```

如果 `MainWindow` 直接依赖具体绘制细节，后面换 VTK 会牵连很多文件。

所以现在结构是：

```text
RenderView
  中央显示模块外壳
  对 MainWindow 暴露 showEmpty() / showBoxGeometry()
  管理标题、尺寸文本、说明文本

RenderCanvas
  旧 2D 绘制后端

VtkRenderCanvas
  当前 VTK 绘制后端
```

也就是说：

```text
MainWindow
  -> RenderView
      -> VtkRenderCanvas
```

这样后面继续升级 VTK 或换成 OCC 转换结果时，外部接口可以尽量不变。

## 9. 为什么接 VTK 时先用 vtkCubeSource？

因为我们现在还没有接 Open CASCADE。

如果一上来就做：

```text
OCC TopoDS_Shape
  -> 三角化
  -> vtkPolyData
  -> vtkActor
```

会同时引入 OCC 和 VTK 两个复杂点。

所以先用 VTK 自带的：

```text
vtkCubeSource
```

来验证 Qt + VTK 嵌入。

当前 VTK 显示链路是：

```text
BoxGeometry
  -> VtkRenderCanvas::showBoxGeometry()
  -> vtkCubeSource
  -> vtkPolyDataMapper
  -> vtkActor
  -> vtkRenderer
  -> QVTKOpenGLNativeWidget
```

对应源码：

```text
src/ui/VtkRenderCanvas.h
src/ui/VtkRenderCanvas.cpp
```

`main.cpp` 中还有一个关键设置：

```cpp
QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
```

这行必须在 `QApplication` 创建前执行。

## 10. 当前 CMake 为什么比较复杂？

因为现在项目同时依赖：

```text
Qt6
VTK
Windows DLL 部署
Debug / Release 两套 VTK
```

当前 Qt 路径：

```text
D:/dev/Qt/6.6.3/msvc2019_64
```

当前 VTK 路径：

```text
Release:
D:/dev/VTK-9.4.2/vtk-9.4.2-qt-release

Debug:
D:/dev/VTK-9.4.2/vtk-9.4.2-qt-debug
```

Debug 构建应该用：

```text
VTK_DIR=D:/dev/VTK-9.4.2/vtk-9.4.2-qt-debug/lib/cmake/vtk-9.4
```

Release 构建应该用：

```text
VTK_DIR=D:/dev/VTK-9.4.2/vtk-9.4.2-qt-release/lib/cmake/vtk-9.4
```

`CMakeLists.txt` 里已经做了默认选择：

```text
Debug 构建优先使用 debug VTK
其他构建默认使用 release VTK
```

VTK 必须保留：

```cmake
vtk_module_autoinit(
    TARGETS MyCAE
    MODULES ${VTK_LIBRARIES}
)
```

否则 VTK 模块可能无法正常初始化。

## 11. 为什么运行时报过 Qt platform plugin 错误？

你运行时报过：

```text
This application failed to start because no Qt platform plugin could be initialized.
```

这个错误不是代码逻辑问题。

原因是 exe 目录缺少：

```text
platforms/qwindowsd.dll
```

Qt Widgets 程序在 Windows 上必须加载这个平台插件。

所以我们在 CMake 中加入了构建后部署：

```text
windeployqt6
```

必须使用 Qt6 自带版本：

```text
D:/dev/Qt/6.6.3/msvc2019_64/bin/windeployqt6.exe
```

不能用 Anaconda PATH 里的 Qt5 `windeployqt.exe`。

如果又遇到这个错误，可以手动执行：

```powershell
D:\dev\Qt\6.6.3\msvc2019_64\bin\windeployqt6.exe --debug --no-translations --no-system-d3d-compiler --no-opengl-sw D:\study\Qt_code\cae\MyCAE\build\Desktop_Qt_6_6_3_MSVC2019_64bit-Debug\MyCAE.exe
```

然后检查：

```text
build/.../platforms/qwindowsd.dll
```

是否存在。

## 12. 当前如何验证功能？

推荐在 Qt Creator 中验证。

步骤：

```text
1. 打开 MyCAE/CMakeLists.txt。
2. 使用 Desktop Qt 6.6.3 MSVC2019 64bit Kit。
3. 重新运行 CMake 配置。
4. 构建并运行。
5. File -> New Project。
6. 选择一个空目录。
7. Geometry -> Create Box。
8. 输入尺寸，例如：
   length = 300
   width  = 120
   height = 80
   unit   = mm
9. 点击 OK。
10. 检查：
    - geometry/box_1.json 已生成
    - 左侧工程树出现 Box_1
    - 右侧属性面板显示尺寸
    - 中央 VTK 视图显示立方体
    - 底部日志输出创建信息
11. 再创建一个尺寸不同的 Box。
12. 点击 Box_1 / Box_2，检查属性面板和 VTK 立方体比例是否同步变化。
13. 关闭软件后重新 Open Project，检查已有 Box 是否能重新加载。
```

## 13. 当前代码结构总览

```text
src/
├── main.cpp
├── project/
│   ├── Project.h
│   ├── ProjectManager.h
│   └── ProjectManager.cpp
├── geometry/
│   ├── BoxGeometry.h
│   ├── GeometryManager.h
│   └── GeometryManager.cpp
└── ui/
    ├── MainWindow.h / MainWindow.cpp
    ├── ProjectTreePanel.h / ProjectTreePanel.cpp
    ├── PropertyPanel.h / PropertyPanel.cpp
    ├── LogPanel.h / LogPanel.cpp
    ├── BoxDialog.h / BoxDialog.cpp
    ├── RenderView.h / RenderView.cpp
    ├── RenderCanvas.h / RenderCanvas.cpp
    └── VtkRenderCanvas.h / VtkRenderCanvas.cpp
```

核心职责再总结一次：

```text
MainWindow
  负责协调。

ProjectManager
  负责工程文件夹和 project.json。

GeometryManager
  负责 Box JSON 的创建和读取。

ProjectTreePanel
  负责工程树显示和选择信号。

PropertyPanel
  负责显示当前对象属性。

RenderView
  负责中央显示区域外壳。

VtkRenderCanvas
  负责真实 VTK 渲染。
```

## 14. 下一步我们应该做什么？

下一步建议做：

```text
阶段 6A：Open CASCADE 环境检查
```

仍然不要一上来写完整 OCC 模块。

先检查：

```text
1. 本机有没有 Open CASCADE。
2. CMake 能不能 find_package(OpenCASCADE)。
3. OCC 编译器版本是否和 Qt6 / VTK 匹配。
4. 是否能写一个最小测试创建 BRepPrimAPI_MakeBox。
```

如果检查通过，再进入：

```text
阶段 6B：用 OCC 创建真实 Box
阶段 6C：保存 BREP / STEP
阶段 7：OCC Shape 转 VTK 显示
```

最终要把当前链路：

```text
BoxGeometry -> vtkCubeSource -> VTK 显示
```

升级成：

```text
Box 参数
  -> OCC 创建 TopoDS_Shape
  -> BREP / STEP 保存
  -> 三角化为 vtkPolyData
  -> VTK 显示
```

## 15. 后续继续开发时的原则

继续保持我们目前的开发节奏：

```text
先跑通最小案例
再解释原理
再扩展功能
一次只引入一个主要复杂点
每一步都更新 step.md
```

不要直接跳到：

```text
完整 CAD 内核
完整网格系统
完整求解器系统
完整后处理系统
```

更稳的路线是：

```text
OCC 环境检查
  -> OCC 最小 Box
  -> OCC 保存 BREP
  -> OCC 转 VTK
  -> Gmsh 环境检查
  -> 最小网格生成
  -> CalculiX 环境检查
  -> 最小求解闭环
```

## 16. 界面中文化记录

当前已将用户直接可见的英文界面文本替换为中文，包括：

```text
菜单栏
工具栏
Dock 面板标题
新建 / 打开工程对话框标题
创建长方体对话框
属性面板字段
状态栏文本
日志输出
错误提示
中央 VTK 显示说明
旧 2D 线框后端尺寸标注
```

没有修改的内容：

```text
类名
函数名
变量名
JSON 字段名
box_*.json 文件名
Box_1 / Box_2 几何对象内部名称
VTK / Qt / CMake 相关英文 API 名称
```

原因是这些内容属于代码接口、文件格式或第三方库 API，直接翻译会影响兼容性和后续开发。

## 17. 阶段 8A / 8B / 8C 数据结构设计

本阶段参考“流体 Pipe 算例说明”，先只建立求解前处理的数据结构，不直接实现完整求解器导出。

算例中的关键数据是：

```text
几何：
  圆柱1：origin=(0, 0, 0), direction=Z, length=0.4, radius=0.05
  圆柱2：origin=(0, -0.4, 0), direction=Y, length=1.5, radius=0.075
  Bool 合并结果：Pipe
  面组：Inlet1 / Inlet2 / Outlet

网格：
  minimumSize=0
  maximumSize=1
  AutoSize=true
  localFaceGroup=pipe.Default
  划分后自动导入网格文件
  Mesh -> Boundary 显示边界

求解：
  -> SIMPLE 求解器
  -> k-Omega SST 湍流模型
  -> 默认边界：Wall
  -> Inlet1：速度入口，值=0.8
  -> Inlet2：压力入口，值=1
  -> Outlet：压力出口
  -> 求解时间：400
  -> 取消求解文件清除

后处理：
  ParaView
```

对应代码结构：

```text
src/solver/Material.h
  阶段 8A：材料数据结构。

src/solver/BoundaryCondition.h
  阶段 8B：边界条件数据结构，描述边界类型和作用目标。

src/solver/Load.h
  阶段 8C：载荷数据结构，描述速度、压力等具体数值。

src/solver/SimulationCase.h
  将几何设置、网格设置、材料、边界条件、载荷、求解器、湍流模型、运行控制和后处理组合成一个算例。

src/solver/PipeCaseExample.h
  按 Pipe 算例创建一个最小示例。
```

当前有意将边界条件和载荷分开：

```text
BoundaryCondition:
  pipe.Inlet1 是速度入口
  pipe.Inlet2 是压力入口
  pipe.Outlet 是压力出口
  pipe.Default 是壁面

Load:
  Inlet1 velocity = 0.8
  Inlet2 pressure = 1
```

这样后续导出求解器文件时，可以按稳定顺序传输：

```text
Material
  -> BoundaryCondition
  -> Load
  -> RunControl
  -> Solver writer
```
