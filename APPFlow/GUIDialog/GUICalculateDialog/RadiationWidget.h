/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   RadiationWidget.h
 * @brief  辐射界面
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-18
 *********************************************************************/
#ifndef _RadiationWidget_H
#define _RadiationWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QAbstractButton;

namespace Ui {
    class RadiationWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFRadiation;
}

namespace GUI
{
    /**
     * @brief  辐射界面
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date   2025-06-18
     */
    class GUICalculateDialogAPI RadiationWidget : public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper      操作器对象
         * @param[i] parent    父对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-18
         */
        RadiationWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-18
         */
        ~RadiationWidget();
        /**
         * @brief    初始化
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-18
         */
        void init();
        /**
         * @brief    显示事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-18
         */
        void showEvent(QShowEvent * event);
        /**
         * @brief    隐藏事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-18
         */
        void resizeEvent(QResizeEvent * event);

    private:
        /**
         * @brief    初始化辐射参数
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-19
         */
        void initRadiationPara();
        /**
         * @brief    初始化辐射模型数据
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-19
         */
        void initModels();
        /**
         * @brief    初始化太阳源数据
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-19
         */
        void initSolarLoad();

    private:
        /**
         * @brief  UI对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-18
         */
        Ui::RadiationWidget* _ui = nullptr;
        /**
         * @brief  求解器运行参数对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-18
         */
        Interface::FITKOFRadiation* _radiationObj = nullptr;
    };
}

#endif
