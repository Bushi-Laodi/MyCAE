/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    PickedDataCalculator.h
 * @brief   拾取数据计算生成类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-07-19
 *********************************************************************/

#ifndef __PICKEDDATACALCULATOR_H__
#define __PICKEDDATACALCULATOR_H__

#include <QObject>

#include "GUIWidgetAPI.h"

// GUI
#include "GUIPickInfo.h"

// 前置声明
class vtkCell;
class vtkIdList;
class vtkIntArray;
class vtkUnstructuredGrid;

namespace GraphData
{
    class PickedData;
}

namespace GraphData
{
    /**
     * @brief   拾取数据计算生成类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-07-19
     */
    class GUIWIDGETAPI PickedDataCalculator : public QObject
    {
        Q_OBJECT
    public:
        /**
         * @brief   构造函数。
         * @param   pickedInfo：拾取信息
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        PickedDataCalculator(GraphData::PickedData *pickedInfo);

        /**
         * @brief   析构函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        ~PickedDataCalculator() = default;

        /**
         * @brief   根据拾取类型与相关信息进行计算，获取实际需要高亮数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void calculate();

    private:
        /**
         * @brief   单独拾取模式数据生成。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void individually();

        /**
         * @brief   框选模式数据生成。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        void byAreaPick();

    private:
        /**
         * @brief   需要计算的拾取数据。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-07-19
         */
        GraphData::PickedData *m_pickedData{nullptr};

    };
}

#endif // __PICKEDDATACALCULATOR_H__
