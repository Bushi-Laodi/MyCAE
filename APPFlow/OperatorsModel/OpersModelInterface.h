/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file OpersModelInterface.h
 * @brief 本模块接口
 * @author LiBaojun (libaojunqd@foxmail.com)
 * @date 2024-04-14
 *
 */
#ifndef __MODELSOPERATOR_INTERFACE_H__
#define __MODELSOPERATOR_INTERFACE_H__

#include "OperatorsModelAPI.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponentInterface.h"


namespace OperModel
{
    /**
     * @brief 模型相关操作器注册，主要用于加载dll
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2024-04-14
     */
    class OperatorsModelAPI OpersModelInterface : public AppFrame::FITKComponentInterface
    {
    public:
        explicit OpersModelInterface() = default;
        virtual ~OpersModelInterface() = default;
        /**
         * @brief 初始化
         * @author LiBaojun (libaojunqd@foxmail.com)
         * @date 2024-04-14
         */
        void init() override;

        /**
        * @brief 获取部件名称，不能重复 TianjianModelOpers
        * @return QString
        * @author LiBaojun (libaojunqd@foxmail.com)
        * @date 2024-03-04
        */
        virtual QString getComponentName() override;
    };


}


#endif
