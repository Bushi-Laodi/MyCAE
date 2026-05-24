/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    OperGraphPreprocess.h
 * @brief   前处理可视化事件操作器。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef _OPERGRAPHPREPROCESS_H__
#define _OPERGRAPHPREPROCESS_H__

#include "OperGraphEvent3D.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace GUIOper
{
    /**
     * @brief   前处理可视化事件处理。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperGraphPreprocess : public OperGraphEvent3D
    {
        Q_OBJECT

    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit OperGraphPreprocess() = default;

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        ~OperGraphPreprocess() = default;

        // 前处理渲染与窗口功能。
        //@{
        /**
         * @brief   根据提供的数据对象ID更新对应可视化对象。（没有则创建）[重写]
         * @param   dataObjId：数据对象ID
         * @param   forceUpdate：是否强制刷新[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateGraph(int dataObjId, bool forceUpdate = false) override;

        /**
         * @brief   根据提供的数据类型创建可视化对象并执行相关逻辑。（没有则创建）[重写]
         *          （此接口用于非数据类可视化对象预览。）
         * @param   type：数据类型
         * @param   param：可视化对象执行操作参数（默认只进行可视化）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-26
         */
        void updateGraphByType(int type, GraphOperParam param = GraphOperParam()) override;

        /**
         * @brief   根据数据对象ID获取模型可视化对象。（没有则不创建）[重写]
         * @param   dataObjId：数据对象ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-23
         */
        Exchange::FITKFluidVTKGraphObject3D* getModelGraphObjectByDataId(int dataObjId) override;

        /**
         * @brief   将现有模型全部开启或关闭半透明显示。[重写]
         * @param   flag：是否开启
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        void setEnableModelTransparent(bool flag) override;

        /**
         * @brief   将现有（边界）网格全部开启或关闭半透明显示。[重写]
         * @param   flag：是否开启
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-08-02
         */
        void setEnableMeshTransparent(bool flag) override;

        /**
         * @brief   隐藏或显示模型。[重写]
         * @param   dataObjId：数据对象ID
         * @param   visibility：是否可见
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-08-02
         */
        void setModelVisible(int dataObjId, bool visibility) override;

        /**
         * @brief   隐藏或显示（边界）网格。[重写]
         * @param   dataObjId：数据对象ID
         * @param   visibility：是否可见
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-08-02
         */
        void setMeshVisible(int dataObjId, bool visibility) override;
        //@}

        // 高亮功能接口。
        //@{
        /**
         * @brief   高亮可视化对象。[重写]
         * @param   dataObjId：数据对象ID
         * @param   info：附加信息[缺省]
         * @param   color：高亮颜色[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        void highlight(int dataObjId, QVariant info = QVariant(), QColor color = QColor()) override;

        /**
         * @brief   根据附加信息高级高亮指定的数据。[重写]
         * @param   dataObjId：数据对象ID
         * @param   indice：附加信息（索引或数据ID）[引用]
         * @param   color：高亮颜色[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        void advHighlight(int dataObjId, QVector<int> & indice, QColor color = QColor()) override;

        /**
         * @brief   取消全部高亮、高级高亮数据的高亮效果。[重写]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        void clearHighlight() override;
        //@}

        // 渲染专用接口。
        //@{
        /**
         * @brief   刷新渲染窗口。[重写]
         * @param   fitView：是否重置相机
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        void reRender(bool fitView = false) override;
        //@}

    };

    Register2FITKOperatorRepo(GraphPreprocess, OperGraphPreprocess);

}  // namespace GUIOper

#endif
