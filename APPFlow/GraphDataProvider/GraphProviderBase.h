/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    GraphProviderBase.h
 * @brief   三维可视化数据管理基类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHPROVIDERBASE_H__
#define __GRAPHPROVIDERBASE_H__

#include <QObject>

#include "GraphDataProviderAPI.h"

#include <QHash>

// 前置声明
namespace Core
{
    class FITKAbstractDataObject;
}

namespace Comp
{
    class FITKGraph3DWindowVTK;
}

namespace Exchange
{
    class FITKFluidVTKGraphObject3D;
}

namespace GraphData
{
    /**
     * @brief   三维可视化数据分类管理类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class GRAPHDATAPROVIDERAPI GraphProviderBase : public QObject
    {
        Q_OBJECT

        // 友元，防止外部手动创建或析构。
        friend class GraphProviderManager;

    public:
        /**
         * @brief   获取当前算例已实例化的可视化对象。[纯虚函数]
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual QList<Exchange::FITKFluidVTKGraphObject3D*> getCurrentGraphObjs() = 0;

        /**
         * @brief   获取当前算例已实例化的当前可见可视化对象。
         * @return  可视化对象列表。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QList<Exchange::FITKFluidVTKGraphObject3D*> getCurrentVisibleGraphObjs();

        /**
         * @brief   获取类名。[虚函数]
         * @return  类名
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual QString getClassName();

        /**
         * @brief   根据数据ID更新可视化对象，没有则跳出。[虚函数]
         * @param   dataId：数据ID
         * @param   info：附加信息[缺省]
         * @return  是否存在该数据ID所对应可视化对象。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual bool updateObjById(int dataId, QVariant info = QVariant());

        /**
         * @brief   更新全部当前可见可视化对象显示状态。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void updateVisibility();

        /**
         * @brief   清除全部管理中的可视化对象高亮与高级高亮效果。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-30
         */
        void clearHighlight();

    protected:
        /**
         * @brief   构造函数。
         * @param   graphWidget：可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        GraphProviderBase(Comp::FITKGraph3DWindowVTK* graphWidget);

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~GraphProviderBase();

        /**
         * @brief   移除数据管理字典中的所有可视化对象数据并析构。
         * @param   hash：数据字典
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void deleteObjsHash(QHash<int, Exchange::FITKFluidVTKGraphObject3D*>& hash);

        /**
         * @brief   移除数据管理字典中的所有可视化对象数据并析构。
         * @param   hash：数据字典
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void deleteObjsHash(QHash<int, QHash<int, Exchange::FITKFluidVTKGraphObject3D*>>& hash);

        /**
         * @brief   通过数据对象、适配器关键字与需要保存到的字典生成或获取可视化对象。
         * @param   adaptorKeyName：适配器关键字
         * @param   objDict：可视化对象字典[引用]
         * @param   dataObj：数据对象
         * @return  可视化对象
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-24
         */
        Exchange::FITKFluidVTKGraphObject3D* getGraphObject(QString adaptorKeyName, QHash<int, Exchange::FITKFluidVTKGraphObject3D*>& objDict, Core::FITKAbstractDataObject* dataObj);

    protected:
        /**
         * @brief   管理器所管理的可视化窗口。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        Comp::FITKGraph3DWindowVTK* m_graphWidget{ nullptr };

        /**
         * @brief   临时预览可视化对象数据字典。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        QHash<int, Exchange::FITKFluidVTKGraphObject3D*> m_previewObjHash = {};

    };
}   // namespace GraphData

#endif // __GRAPHPROVIDERBASE_H__
