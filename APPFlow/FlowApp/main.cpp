/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "MainWindowGenerator.h"
#include "GlobalDataFactory.h"
#include "ComponentFactory.h"
#include "CommandLineHandler.h".
#include "SignalProcessor.h"
#include "FlowAPPSettings.h"
#include "WorkBenchHandler.h"
#include "SystemChecker.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_WIN
/**
 * @brief 程序的主入口函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数值数组
 * @return int 程序执行的返回状态
 * @author YanZhiHui(chanyuantiandao@126.com)
 * @date 2024-04-19
 */
int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    //这个是Windows平台用来获取屏幕宽度的代码，必须在QApplication实例化之前使用
    //因为在qApplication实例初始化之前，QGuiApplication::screens();无法使用。
    //qreal  cx = GetSystemMetrics(SM_CXSCREEN);
    //qreal scale = cx / 1920;
    //qputenv("QT_SCALE_FACTOR", QString::number(scale).toLatin1());
#endif // Q_WIN
    //控制图片缩放质量
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 初始化应用框架
    AppFrame::FITKApplication app(argc, argv);

#ifdef Q_OS_LINUX
    qputenv("PYTHONHOME", app.applicationDirPath().toUtf8() + "/../Python37");

    QStringList pythonPaths;
    pythonPaths << app.applicationDirPath() + "/../Python37/bin";
    pythonPaths << app.applicationDirPath() + "/../Python37/lib";
    pythonPaths << app.applicationDirPath() + "/../Python37/lib/python3.7";
    pythonPaths << app.applicationDirPath() + "/../Python37/lib/python3.7/site-packages";
    pythonPaths << app.applicationDirPath() + "/../Python37/lib/python3.7/lib-dynload";
    qputenv("PYTHONPATH", pythonPaths.join(":").toUtf8());
#endif
    app.checkSystem(new SystemChecker);
   // 注册程序的主要组件和设置
    app.regMainWindowGenerator(new MainWindowGenerator); // 注册主窗口生成器
    app.regGlobalDataFactory(new GlobalDataFactory);     // 注册全局数据工厂
    app.regComponentsFactory(new ComponentFactory);      // 注册组件工厂
    app.regAppSettings(new FlowAPPSettings);             // 注册应用结构设置
    app.regCommandLineHandler(new CommandLineHandler);   // 注册命令行处理
    app.addGolbalSignalProcesser(new SignalProcessor);   // 信号处理器
    app.regWorkBenchHandler(new FlowAppWorkBenchHandler); //workbench处理
    /*
    app.regAppSettings(new StructuralSettings);          // 注册应用结构设置
    // 注册全局信号处理器
    app.addGolbalSignalProcesser(new ThreadTaskProcessHandler);
    //注册自定义事件处理
    app.addEventFilter(new MouseKeyEventFilter);*/
    // 运行应用程序的消息循环
    return app.exec();
}

