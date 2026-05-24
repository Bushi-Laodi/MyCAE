/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file GeometryWidgetBase.h
 * @brief 几何界面基类
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef GeometryWidgetBase_H_
#define GeometryWidgetBase_H_

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Interface {
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class MainWindow;
    /**
     * @brief 几何界面基类
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIGeometryDialogAPI GeometryWidgetBase : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Geometry Widget Base object
         * @param[i]  obj            几何对象
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GeometryWidgetBase(Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Geometry Widget Base object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual ~GeometryWidgetBase();
        /**
         * @brief    获取Default面组
         * @return   QList<int>    Default面组ID
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        static QList<int> getDefaultFaceGroup(Interface::FITKAbsGeoCommand* geoObj);
        /**
         * @brief    创建Default面组
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        static void createDefaultFaceGroup(Interface::FITKAbsGeoCommand* geoObj);
        /**
         * @brief 获取当前模型数据对象
         * @return Interface::FITKAbsGeoCommand*
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    protected:
        /**
         * @brief 显示事件
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void showEvent(QShowEvent *event) override;
        /**
         * @brief 关闭事件
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void closeEvent(QCloseEvent* event) override;
        /**
         * @brief 几何、网格半透明
         * @param[i]  geo            是否半透明
         * @param[i]  mesh           是否半透明
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void transparency(bool geo = true, bool mesh = true);
        /**
         * @brief 创建几何网格参数对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void createMeshSizeGeo();
    protected:
        /**
         * @brief 主窗口对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MainWindow* _mainWin = nullptr;
        /**
         * @brief 几何数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbsGeoCommand* _obj = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
