/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    OperGraphPickPreview.h
 * @brief   拾取预选高亮操作器。
 *
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-07-19
 *********************************************************************/

#ifndef __OPERGRAPHPICKPREVIEW_H__
#define __OPERGRAPHPICKPREVIEW_H__

#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

 // 前置声明
namespace Exchange
{
    class FITKFluidVTKGraphObject3D;
    class FITKFluidVTKGraphObjectPickPreview;
}

namespace GraphData
{
    class PickedDataProvider;
}

namespace GUIOper
{
    /**
     * @brief   高亮可视化事件处理。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-07-19
     */
    class OperGraphPickPreview : public EventOper::GraphInteractionOperator
    {
    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        explicit OperGraphPickPreview();

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        ~OperGraphPickPreview();

        /**
         * @brief   拾取单个节点或单元接口。[重写]
         * @param   graphWindow：可视化窗口
         * @param   actor：演员
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor2D* actor) override;

        /**
         * @brief   预选单个节点或单元接口。[重写]
         * @param   graphWindow：可视化窗口
         * @param   actor：演员
         * @param   index：索引
         * @param   pickedWorldPos：拾取的世界坐标
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor* actor, int index, double* pickedWorldPos) override;

        /**
         * @brief   清除预选高亮对象渲染。[重写]
         * @param   graphWindow：可视化窗口[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void clear(Comp::FITKGraph3DWindowVTK* graphWindow = nullptr) override;

    private:
        /**
         * @brief   添加可视化对象至三维窗口。
         * @param   obj：可视化对象
         * @param   graphWidget：三维窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void addGraphObjectToWidget(Exchange::FITKFluidVTKGraphObject3D* obj, Comp::FITKGraph3DWindowVTK* graphWidget);

    private:
        /**
         * @brief   预选高亮可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        Exchange::FITKFluidVTKGraphObjectPickPreview* m_graphObjPreview{ nullptr };

        /**
         * @brief   拾取信息管理器。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        GraphData::PickedDataProvider* m_pickedProvider{ nullptr };

    };

    Register2FITKOperatorRepo(GraphPickPreview, OperGraphPickPreview);

}  // namespace GUIOper

#endif // __OPERGRAPHPICKPREVIEW_H__
