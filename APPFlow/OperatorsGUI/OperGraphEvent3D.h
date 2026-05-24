/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    OperGraphEvent3D.h
 * @brief   三维可视化事件操作器抽象类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __OPERGRAPHEVENT3D_H__
#define __OPERGRAPHEVENT3D_H__

#include "OperatorsInterface/GraphEventOperator.h"

// 前置声明
namespace Comp
{
    class FITKGraph3DWindowVTK;
}

namespace Exchange
{
    class FITKFluidVTKGraphObject3D;
}

namespace GUIOper
{
    /**
     * @brief   可视化事件操作器。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperGraphEvent3D : public EventOper::GraphEventOperator
    {
        Q_OBJECT

    protected:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit OperGraphEvent3D() = default;

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~OperGraphEvent3D() = default;

        /**
         * @brief   获取三维可视化窗口。
         * @return  可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        Comp::FITKGraph3DWindowVTK* getGraphWidget();

        /**
         * @brief   添加可视化对象至三维窗口。
         * @param   obj：可视化对象
         * @param   graphWidget：三维窗口
         * @param   fitView：是否重置视角[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void addGraphObjectToWidget(Exchange::FITKFluidVTKGraphObject3D* obj, Comp::FITKGraph3DWindowVTK* graphWidget, bool fitView = false);

        /**
         * @brief   通过数据对象ID获取可视化对象。（没有则创建）
         * @param   dataObjId：数据对象ID
         * @return  可视化对象列表
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getGraphObjectsByDataId(int dataObjId);

    };

}  // namespace GUIOper

#endif // __OPERGRAPHEVENT3D_H__
