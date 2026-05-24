/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    OperGraphPick.h
 * @brief   拾取高亮操作器。
 *
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-07-19
 *********************************************************************/

#ifndef __OPERGRAPHPICK_H__
#define __OPERGRAPHPICK_H__

#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

 // 前置声明
class vtkDataSet;

namespace Exchange
{
    class FITKFluidVTKGraphObject3D;
    class FITKFluidVTKGraphObjectPick;
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
    class OperGraphPick : public EventOper::GraphInteractionOperator
    {
    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        explicit OperGraphPick();

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        ~OperGraphPick();

        /**
         * @brief   拾取高亮单个节点或单元接口。[重写]
         * @param   graphWindow：可视化窗口
         * @param   actor：演员
         * @param   index：索引
         * @param   pickedWorldPos：拾取的世界坐标
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor* actor, int index, double* pickedWorldPos) override;

        /**
         * @brief   框选拾取多个节点或单元接口。[重写]
         * @param   graphWindow：可视化窗口
         * @param   actors：被框选到的演员列表
         * @param   cutPlane：框选时的裁切平面
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void picked(Comp::FITKGraph3DWindowVTK* graphWindow, QList<vtkActor*> actors, vtkPlanes* cutPlane) override;

        /**
         * @brief   拾取单个节点或单元接口。[重写]
         * @param   graphWindow：可视化窗口
         * @param   actor：演员
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor2D* actor) override;

        /**
         * @brief   清除预选高亮对象渲染。[重写]
         * @param   graphWindow：可视化窗口[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void clear(Comp::FITKGraph3DWindowVTK* graphWindow = nullptr) override;

        /**
         * @brief   设置拾取颜色。[重写]
         * @param   color：颜色
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setPickedColor(QColor color) override;

        /**
         * @brief   根据当前拾取对象类型隐藏或显示部分演员或修改可拾取性。[重写]
         * @param   pickObjType：拾取对象类型
         * @param   pickMethod：拾取方法
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setActorStateByPickInfo(int pickObjType, int pickMethod, int dataObjId = -1) override;

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
         * @brief   拾取高亮可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        Exchange::FITKFluidVTKGraphObjectPick* m_graphObjPick{ nullptr };

        /**
         * @brief   拾取信息管理器。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        GraphData::PickedDataProvider* m_pickedProvider{ nullptr };
    };

    Register2FITKOperatorRepo(GraphPick, OperGraphPick);

}  // namespace GUIOper

#endif // __OPERGRAPHPICK_H__
