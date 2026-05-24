/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    PickedDataProvider.h
 * @brief   拾取数据管理类。（并未三维可视化对象管理）
 *
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-04-17
 *********************************************************************/

#ifndef __PICKEDDATAPROVIDER_H__
#define __PICKEDDATAPROVIDER_H__

#include <QObject>

#include "GUIWidgetAPI.h"

#include "FITK_Interface/FITKInterfaceModel/FITKModelEnum.h"

#include <QHash>

 // 前置声明
class vtkActor;
class vtkActor2D;
class vtkPlanes;
class vtkDataSet;
class vtkPoints;
class vtkUnstructuredGrid;

namespace AppFrame
{
    class FITKKeyMouseStates;
}

namespace GraphData
{
    class PickedData;
}

namespace GraphData
{
    /**
     * @brief   拾取数据管理类。（并未三维可视化对象管理）
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    class GUIWIDGETAPI PickedDataProvider : public QObject
    {
        Q_OBJECT
    public:
        /**
         * @brief   获取单例。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-17
         */
        static PickedDataProvider* getInstance();

        /**
         * @brief   手动销毁。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-17
         */
        static void Delete();

        /**
         * @brief   手动添加数据，模拟拾取。
         * @param   type：数据类型
         * @param   dataObjId：数据对象ID
         * @param   indice：数据索引[引用]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-23
         */
        void addDataManually(Interface::FITKModelEnum::FITKModelSetType type, int dataObjId, QList<int> & indice);

        /**
         * @brief   清空拾取数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        void clearPickedData();

        /**
         * @brief   清空预选数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-30
         */
        void clearPickedDataPreview();

        /**
         * @brief   获取预选可视化需要的VTK数据。
         * @return  数据集
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-23
         */
        vtkDataSet* getPickedPreviewDataSet();

        /**
         * @brief   获取合并的需要可视化的拾取VTK数据。
         * @return  数据集
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        vtkDataSet* getPickedDataSet();

        /**
         * @brief   生成预选或拾取数据。
         * @param   actor：拾取演员
         * @param   index：拾取索引
         * @param   pickedWorldPos：拾取的世界坐标
         * @param   isPreview：是否为预选
         * @return  预选或拾取数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-24
         */
        PickedData* getPickedData(vtkActor* actor, int index, double* pickedWorldPos, bool isPreview = false);

        /**
         * @brief   生成预选或拾取数据。
         * @param   actor：拾取演员
         * @param   isPreview：是否为预选
         * @return  预选或拾取数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-24
         */
        PickedData* getPickedData(vtkActor2D* actor, bool isPreview = false);

        /**
         * @brief   保存预选数据。
         * @param   actor：拾取演员
         * @param   index：拾取索引
         * @param   pickedWorldPos：拾取的世界坐标
         * @return  是否需要刷新
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        bool addPreviewPicked(vtkActor* actor, int index, double* pickedWorldPos);

        /**
         * @brief   保存预选数据。
         * @param   actor：拾取演员
         * @return  是否需要刷新
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-05-16
         */
        bool addPreviewPicked(vtkActor2D* actor);

        /**
         * @brief   添加单次点击拾取数据。
         * @param   actor：拾取演员
         * @param   index：拾取索引
         * @param   pickedWorldPos：拾取的世界坐标
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        void addPicked(vtkActor* actor, int index, double* pickedWorldPos);

        /**
         * @brief   保存拾取数据。
         * @param   actor：拾取演员
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-05-16
         */
        void addPicked(vtkActor2D* actor);

        /**
         * @brief   添加拾取多个演员的节点或单元信息。
         * @param   actors：拾取演员列表
         * @param   cutPlane：拾取器裁切面
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-19
         */
        void addPicked(QList<vtkActor*> actors, vtkPlanes* cutPlane);

        /**
         * @brief   获取拾取信息列表。
         * @return  拾取信息
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-19
         */
        QList<PickedData*> getPickedList();

    signals:
        /**
         * @brief   信号 - 有数据被拾取到。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-05-23
         */
        void sig_dataPicked();

    protected:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-17
         */
        PickedDataProvider();

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-17
         */
        ~PickedDataProvider();

        /**
         * @brief   根据键盘事件处理拾取数据。
         * @param   data：拾取数据
         * @param   isAreaPick：是否为框选（框选处理数据时无需自动清空拾取信息）
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-19
         */
        void dealPickedData(PickedData* data, bool isAreaPick = false);

    private:
        /**
         * @brief   根据当前预选信息重新生成VTK数据集进行可视化。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-23
         */
        void generatePickedPreviewDataSet();

        /**
         * @brief   根据当前拾取信息重新生成VTK数据集进行可视化。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        void generatePickedDataSet();

        /**
         * @brief   找到相同模型点拾取数据，不存在则返回空。
         * @param   data：查找数据的依据
         * @return  找到的数据
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-19
         */
        PickedData* getSameModelPickData(PickedData* data);

    private:
        /**
         * @brief   实例。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-17
         */
        static PickedDataProvider* s_instance;

        /**
         * @brief   系统设置单例。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        AppFrame::FITKKeyMouseStates* m_settings{ nullptr };

        /**
         * @brief   所有拾取信息。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        QList<PickedData*> m_pickedDataList;

        /**
         * @brief   预选高亮数据信息。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-24
         */
        PickedData* m_pickedPreviewData{ nullptr };

        /**
         * @brief   预选可视化所需VTK数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-23
         */
        vtkUnstructuredGrid* m_dataSetPreview{ nullptr };

        /**
         * @brief   合并的可视化所需拾取VTK数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-04-18
         */
        vtkUnstructuredGrid* m_dataSetPicked{ nullptr };

    };
}

#endif // __PICKEDDATAPROVIDER_H__
