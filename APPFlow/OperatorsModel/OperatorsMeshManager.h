/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperatorsMeshManager.h
 * @brief 网格相关操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsMeshManager_H
#define _OperatorsMeshManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 网格相关操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsMeshManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Mesh Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsMeshManager() = default;
        /**
         * @brief Destroy the Operators Mesh Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsMeshManager() = default;
        /**
         * @brief 执行
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool execGUI();
        /**
         * @brief 执行结果处理
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool execProfession();
    protected:
        /**
         * @brief 读取网格
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void readMesh();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionClearMesh, OperatorsMeshManager);
    Register2FITKOperatorRepo(actionMesh, OperatorsMeshManager);
    Register2FITKOperatorRepo(actionMeshEdit, OperatorsMeshManager);
}

#endif