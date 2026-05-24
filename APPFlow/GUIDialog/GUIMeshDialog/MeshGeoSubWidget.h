/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file MeshGeoSubWidget.h
 * @brief 几何相关的网格区域尺寸子界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _MeshGeoSubWidget_H
#define _MeshGeoSubWidget_H

#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class MeshGeoSubWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKRegionMeshSizeGeom;
    class FITKRegionGeometryRefine;
    class FITKRegionMeshSizeManager;
    class FITKRegionGeometryRefineManager;
    class FITKAbsGeoCommand;
}

namespace GUI
{
    /**
     * @brief 几何相关的网格区域尺寸子界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class MeshGeoSubWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Geo Sub Widget object
         * @param[i]  geoID          几何id
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshGeoSubWidget(Interface::FITKAbsGeoCommand* geoObj, EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Mesh Geo Sub Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshGeoSubWidget();
        /**
         * @brief 获取当前的几何id
         * @return int id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        int getObjID();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void updateWidget();
    private slots:
        /**
         * @brief 最小细化参数编辑事件
         * @param[i]  arg1           数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_spinBox_Min_valueChanged(int arg1);
        /**
         * @brief 最大细化参数编辑事件
         * @param[i]  arg1           数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_spinBox_Max_valueChanged(int arg1);
        void on_checkBox_Mesh_clicked();

        void on_checkBox_Refine_clicked();

        void on_checkBox_Limit_clicked();

        void on_comboBox_RefineModel_currentIndexChanged(int index);

        void on_doubleSpinBox_Distance_valueChanged(double arg1);

        void on_spinBox_Level_valueChanged(int arg1);

        void on_comboBox_Gap_currentIndexChanged(int index);

        void on_spinBox_NoCell_valueChanged(int arg1);

        void on_spinBox_MaxGap_valueChanged(int arg1);
    private:
        /**
         * @brief Set the Data To Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
        /**
         * @brief    get the data from widget
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void getDataFromWidget();
    private:
        /**
         * @brief  几何相关网格局部区域尺寸管理器
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Interface::FITKRegionMeshSizeManager* _meshSizeManager = nullptr;
        /**
         * @brief  几何划分网格优化区域对象管理器
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Interface::FITKRegionGeometryRefineManager* _geoRefineManager = nullptr;
        /**
         * @brief 几何相关网格局部区域尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKRegionMeshSizeGeom* _geoMeshSize = nullptr;
        /**
         * @brief  几何划分网格优化区域对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Interface::FITKRegionGeometryRefine* _geoRefine = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::MeshGeoSubWidget* _ui = nullptr;
        /**
         * @brief 几何对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbsGeoCommand* _geoObj = nullptr;
    };
}

#endif
