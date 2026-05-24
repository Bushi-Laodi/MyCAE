/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperatorsImportManager.h
 * @brief 文件导入操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsImportManager_H
#define _OperatorsImportManager_H

#include "OperManagerBase.h"
#include "FITK_Kernel/FITKCore/FITKThreadTask.h"

namespace ModelOper
{
    /**
     * @brief 导入类型
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    enum class ImportType {
        ImportNone,                ///空
        ImportGeo,                 ///几何
        ImportMesh,                ///网格
        ImportOpenFoamMesh,        ///OpenFoam网格
    };
    /**
     * @brief 文件导入操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsImportManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Import Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsImportManager();
        /**
         * @brief Destroy the Operators Import Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsImportManager();
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
    private slots:
        ;
        /**
         * @brief 几何导入完成槽函数
         * @param[i]  result         是否成功
         * @param[i]  objID          几何id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotGeoImportFinish(bool result, int objID);
        /**
         * @brief 网格导入完成槽函数
         * @param[i]  result         是否成功
         * @param[i]  objID          网格id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotMeshImportFinish(bool result, int objID);
        /**
         * @brief    OpenFoam网格文件导入槽函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        void slotFoamMeshInportFinish();
    };

    /**
     * @brief 导入文件读取线程
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class ImportReadThread :public Core::FITKThreadTask
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Import Read Thread object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ImportReadThread() = default;
        /**
         * @brief Destroy the Import Read Thread object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~ImportReadThread() = default;
        /**
         * @brief 执行函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void run();
    signals:
        ;
        /**
         * @brief 导入完成信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void sigImportFinish(bool, int);
    public:
        /**
         * @brief 类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ImportType _type = ImportType::ImportNone;
        /**
         * @brief 文件名称
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QString _fileName = "";
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionImportGeometry, OperatorsImportManager);
    Register2FITKOperatorRepo(actionImportMesh, OperatorsImportManager);
    Register2FITKOperatorRepo(actionImportOpenFoamMesh, OperatorsImportManager);
}

#endif