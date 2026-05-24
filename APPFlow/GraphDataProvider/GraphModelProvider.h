/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    GraphModelProvider.h
 * @brief   三维模型可视化数据管理。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHMODELPROVIDER_H__
#define __GRAPHMODELPROVIDER_H__

#include "GraphProviderBase.h"

#include "GraphDataProviderAPI.h"

#include <QHash>

// 前置声明

namespace GraphData
{
    /**
     * @brief   三维可视化数据分类管理类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class GRAPHDATAPROVIDERAPI GraphModelProvider : public GraphProviderBase
    {
        Q_OBJECT

        // 友元，防止外部手动创建或析构。
        friend class GraphProviderManager;

    public:
        /**
         * @brief   获取当前算例已实例化的可视化对象。
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getCurrentGraphObjs() override;

        /**
         * @brief   获取类名。[重写]
         * @return  类名
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QString getClassName() override;

        /**
         * @brief   根据数据ID获取模型（几何）可视化对象。（没有则创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        Exchange::FITKFluidVTKGraphObject3D* getModelGraphObject(int dataObjId);

        /**
         * @brief   获取全部模型（几何）可视化对象。（没有则不创建）
         * @return  可视化对象列表
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getAllModelGraphObjects();

        /**
         * @brief   根据数据ID获取边界网格可视化对象。（没有则创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象（边界网格数据ID）
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        Exchange::FITKFluidVTKGraphObject3D* getBoundMeshGraphObject(int dataObjId);

        /**
         * @brief   根据数据ID获取流体网格下的全部边界网格可视化对象。（没有则创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象（流体网格数据ID）
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getFuildBoundMeshGraphObjects(int dataObjId);

        /**
         * @brief   获取全部（边界）网格可视化对象。（没有则不创建）
         * @return  可视化对象列表
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-08-02
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getAllMeshGraphObjects();

        /**
         * @brief   根据数据ID获取流体域形状可视化对象。（没有则创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象（流体域形状数据ID）
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-25
         */
        Exchange::FITKFluidVTKGraphObject3D* getRegionMeshGraphObject(int dataObjId);

        /**
         * @brief   根据数据ID获取对应可视化对象。（没有则不创建）
         * @param   dataObjId：数据ID
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-23
         */
        Exchange::FITKFluidVTKGraphObject3D* getCurrentGraphObjByDataId(int dataObjId);

        /**
         * @brief   根据数据ID更新可视化对象，没有则跳出。
         * @param   dataObjId：数据ID
         * @param   info：附加信息[缺省]
         * @return  是否存在该数据ID所对应可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        bool updateObjById(int dataObjId, QVariant info = QVariant());

        /**
         * @brief   设置当前所有实例化模型可视化对象是否可拾取点。
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setVertPickable(int dataObjId = -1);

        /**
         * @brief   设置当前所有实例化模型可视化对象是否可拾取线。
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setEdgePickable(int dataObjId = -1);

        /**
         * @brief   设置当前所有实例化模型可视化对象是否可拾取面。
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setFacePickable(int dataObjId = -1);

        /**
         * @brief   设置当前所有实例化模型可视化对象是否可拾取体。
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setSolidPickable(int dataObjId = -1);

        /**
         * @brief   关闭模型拾取状态。
         * @param   dataObjId：生效数据ID（-1则对所有数据生效）[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void setNonePickable(int dataObjId = -1);

    private:
        /**
         * @brief   构造函数。
         * @param   graphWidget：可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphModelProvider(Comp::FITKGraph3DWindowVTK* graphWidget);

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        ~GraphModelProvider();

    private:
        /**
         * @brief   模型（几何）可视化对象字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QHash<int, Exchange::FITKFluidVTKGraphObject3D*> m_modelObjHash;

        /**
         * @brief   边界网格可视化对象字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        QHash<int, Exchange::FITKFluidVTKGraphObject3D*> m_boundMeshObjHash;

        /**
         * @brief   流体域形状可视化对象字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-25
         */
        QHash<int, Exchange::FITKFluidVTKGraphObject3D*> m_regionMeshObjHash;

    };
}   // namespace GraphData

#endif // __GRAPHMODELPROVIDER_H__
