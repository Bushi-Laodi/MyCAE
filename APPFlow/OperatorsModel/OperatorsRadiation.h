/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   OperatorsRadiation.h
 * @brief  求解器流场辐射操作器
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-18
 *********************************************************************/
#ifndef _OperatorsRadiation_H
#define _OperatorsRadiation_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief  求解器流场辐射操作器
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date   2025-06-18
     */
    class OperatorsRadiation :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Radiation object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        OperatorsRadiation();
        /**
         * @brief Destroy the Operators Radiation object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2025-06-18
         */
        ~OperatorsRadiation();
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
    Register2FITKOperatorRepo(actionRadiationEdit, OperatorsRadiation);
}

#endif