/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperatorsGeoCubeManager.h
 * @brief 立方体几何操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsGeoCubeManager_H
#define _OperatorsGeoCubeManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 立方体几何操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsGeoCubeManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Geo Cube Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsGeoCubeManager();
        /**
         * @brief Destroy the Operators Geo Cube Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsGeoCubeManager();
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
        /**
         * @brief 其他事件执行
         * @param[i]  index          事件索引（自定义）
         * @param[i]  value          其他数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        void eventProcess(int index, QVariant value = QVariant()) override;
    private slots:
        ;
        /**
         * @brief 基点选择槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotReselectBasePoint();
        /**
         * @brief 面组选择槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionGeoCubeCreate, OperatorsGeoCubeManager);
    Register2FITKOperatorRepo(actionGeoCubeEdit, OperatorsGeoCubeManager);
    Register2FITKOperatorRepo(actionGeoCubeDelete, OperatorsGeoCubeManager);
    Register2FITKOperatorRepo(actionGeoCubeRename, OperatorsGeoCubeManager);
}

#endif