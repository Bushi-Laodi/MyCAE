/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file MainWindowGenerator.h
 * @brief 主窗口生成器类
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
#ifndef __MAINWINDOWGENERATOR__
#define __MAINWINDOWGENERATOR__

#include "FITK_Kernel/FITKAppFramework/FITKAbstractMainWinGenerator.h"

/**
 * @brief 主窗口生成器类
 * @note 该类继承自`AppFrame::FITKAbstractMainwindowGenerator`，负责生成应用程序的主窗口。
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
class MainWindowGenerator : public AppFrame::FITKAbstractMainwindowGenerator
{
public:
    /**
     * @brief 构造函数
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    MainWindowGenerator();
    /**
     * @brief 析构函数（默认实现）
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    ~MainWindowGenerator() = default;
    /**
     * @brief 必须重写的方法，用于生成主窗口实例
     * @return 生成的QWidget*类型的主窗口指针
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    QWidget *genMainWindow() override;
};

#endif // !__MAINWINDOWGENERATOR__