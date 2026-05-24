/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file ComamdLineHandler.h
 * @brief 命令行处理器
 * @author LiBaojun (libaojunqd@foxmail.com)
 * @date 2024-04-06
 *
 */
#ifndef __COMMANDLINEHANDLER_H__
#define __COMMANDLINEHANDLER_H__

#include "FITK_Kernel/FITKAppFramework/FITKCommandLineHandler.h"

/**
 * @brief 自定义命令行处理器
 * @author LiBaojun (libaojunqd@foxmail.com)
 * @date 2024-04-06
 */
class CommandLineHandler : public AppFrame::FITKCommandLineHandler
{
public:
    /**
     * @brief Construct a new Command Line Handler object
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2024-04-06
     */
    explicit CommandLineHandler() = default;
    /**
     * @brief Destroy the Command Line Handler object
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2024-04-06
     */
    virtual ~CommandLineHandler() = default;
   /**
    * @brief 执行命令行
    * @author LiBaojun (libaojunqd@foxmail.com)
    * @date 2024-04-06
    */
    void exec() override;

private:

};


#endif

