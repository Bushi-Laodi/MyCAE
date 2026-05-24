/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   OperatorsThermo.h
 * @brief  求解器流场热物理属性操作器
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-18
 *********************************************************************/
#ifndef _OperatorsThermo_H
#define _OperatorsThermo_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器流场热物理属性操作器
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date 2025-06-18
     */
    class OperatorsThermo :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Thermo object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        OperatorsThermo();
        /**
         * @brief Destroy the Operators Thermo object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        ~OperatorsThermo();
        /**
         * @brief 执行
         * @return true 成功
         * @return false 失败
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        virtual bool execGUI();
        /**
         * @brief 执行结果处理
         * @return true 成功
         * @return false 失败
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionThermoEdit, OperatorsThermo);
}

#endif