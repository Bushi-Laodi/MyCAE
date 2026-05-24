/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file SystemChecker.h
 * @brief 系统检查器
 * @author libaojun (libaojunqd@foxmail.com)
 * @date 2024-09-18
 * 
 */
#ifndef _SYSTEM_CHECKER____H___
#define _SYSTEM_CHECKER____H___
 

#include "FITK_Kernel/FITKAppFramework/FITKAbstractSysChecker.h"

/**
 * @brief 系统检查器
 * @author libaojun (libaojunqd@foxmail.com)
 * @date 2024-09-18
 */
class SystemChecker : public AppFrame::FITKAbstractSysChecker
{
public: 
    /**
     * @brief Construct a new System Checker object
     * @author libaojun (libaojunqd@foxmail.com)
     * @date 2024-09-18
     */
    explicit SystemChecker() = default;
    /**
     * @brief Destroy the System Checker object
     * @author libaojun (libaojunqd@foxmail.com)
     * @date 2024-09-18
     */
    virtual ~SystemChecker() = default;
     /**
         * @brief 进行检查，返回错误信息
         * @return QStringList 错误信息
         * @author libaojun (libaojunqd@foxmail.com)
         * @date 2024-09-18
         */
    virtual QStringList check() override;

};


#endif
