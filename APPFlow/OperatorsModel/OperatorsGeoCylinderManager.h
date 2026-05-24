/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperatorsGeoCylinderManager.h
 * @brief 圆柱型几何操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsGeoCylinderManager_H
#define _OperatorsGeoCylinderManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 圆柱型几何操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsGeoCylinderManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Geo Cylinder Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsGeoCylinderManager();
        /**
         * @brief Destroy the Operators Geo Cylinder Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsGeoCylinderManager();
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
        void eventProcess(int index, QVariant value) override;
    private slots:
        ;
        /**
         * @brief 原点选择事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotReselectOriginPoint();
        /**
         * @brief 面组选择事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionGeoCylinderCreate, OperatorsGeoCylinderManager);
    Register2FITKOperatorRepo(actionGeoCylinderEdit, OperatorsGeoCylinderManager);
    Register2FITKOperatorRepo(actionGeoCylinderDelete, OperatorsGeoCylinderManager);
    Register2FITKOperatorRepo(actionGeoCylinderRename, OperatorsGeoCylinderManager);
}

#endif