/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    GraphMarkProvider.h
 * @brief   三维模型可视化数据管理。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-07-26
 *********************************************************************/

#ifndef __GRAPHMARKPROVIDER_H__
#define __GRAPHMARKPROVIDER_H__

#include "GraphProviderBase.h"

#include "GraphDataProviderAPI.h"

#include <QHash>

// 前置声明

namespace GraphData
{
    /**
     * @brief   三维可视化数据分类管理类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-07-26
     */
    class GRAPHDATAPROVIDERAPI GraphMarkProvider : public GraphProviderBase
    {
        Q_OBJECT

        // 友元，防止外部手动创建或析构。
        friend class GraphProviderManager;

    public:
        /**
         * @brief   获取当前算例已实例化的可视化对象。
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getCurrentGraphObjs() override;

        /**
         * @brief   获取类名。[重写]
         * @return  类名
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        QString getClassName() override;

        /**
         * @brief   根据数据类型获取可视化对象。（没有则创建）
         * @param   type：数据对象类型
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        Exchange::FITKFluidVTKGraphObject3D* getGraphObjectByType(int type);

        /**
         * @brief   根据数据ID获取全部材料点可视化对象。（没有则创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        Exchange::FITKFluidVTKGraphObject3D* getMaterialPointsGraphObject();

    private:
        /**
         * @brief   构造函数。
         * @param   graphWidget：可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        GraphMarkProvider(Comp::FITKGraph3DWindowVTK* graphWidget);

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        ~GraphMarkProvider();

    private:
        /**
         * @brief   临时标识符可视化对象字典。（键为数据类型）
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        QHash<int, Exchange::FITKFluidVTKGraphObject3D*> m_tempTypeObjHash;

    };
}   // namespace GraphData

#endif // __GRAPHMARKPROVIDER_H__
