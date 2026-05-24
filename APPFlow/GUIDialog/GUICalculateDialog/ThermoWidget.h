/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   ThermoWidget.h
 * @brief  热属性界面
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-18
 *********************************************************************/
#ifndef _ThermoWidget_H
#define _ThermoWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QAbstractButton;

namespace Ui {
    class ThermoWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFThermo;
    class FITKOFThermoPhysicalProp;
}

namespace Core {
    class FITKParameter;
}

class QVBoxLayout;

namespace GUI
{
    /**
     * @brief  求解器运行界面
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date   2024-09-02
     */
    class GUICalculateDialogAPI ThermoWidget : public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper      操作器对象
         * @param[i] parent    父对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        ThermoWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        ~ThermoWidget();
        /**
         * @brief    初始化
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void init();
        /**
         * @brief    显示事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void showEvent(QShowEvent * event);
        /**
         * @brief    隐藏事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void resizeEvent(QResizeEvent * event);

    private:
        /**
         * @brief    初始化tab界面
         * @param[i] w 
         * @param[i] thermoPhyProp 
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-20
         */
        void initTabWidget(QWidget* w, Interface::FITKOFThermoPhysicalProp* thermoPhyProp);
        /**
         * @brief    初始化Specie
         * @param[i] para 
         * @param[i] pLayout 
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-20
         */
        void initSpecieWidget(Core::FITKParameter * para, QVBoxLayout* pLayout);
        /**
         * @brief    初始化Layout数据
         * @param[i] w 页的窗口
         * @param[i] modelType 模型类型 0:Thermodynamics 1:Transport 2:Equation Of State
         * @param[i] para 参数数据
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-20
         */
        void initLayout(QWidget* w, int modelType, Core::FITKParameter *para);

    private:
        /**
         * @brief  UI对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2024-09-02
         */
        Ui::ThermoWidget* _ui = nullptr;
        /**
         * @brief  求解器运行参数对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2024-09-02
         */
        Interface::FITKOFThermo* _thermoObj = nullptr;
        /**
         * @brief  Thermodynamics垂直布局
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-20
         */
        QHash<QWidget*, QVBoxLayout*> _thermodynamicsVBoxLayout{};
        /**
         * @brief  Transport垂直布局
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-20
         */
        QHash<QWidget*, QVBoxLayout*> _transportVBoxLayout{};
        /**
         * @brief  Equation Of State垂直布局
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2025-06-20
         */
        QHash<QWidget*, QVBoxLayout*> _equationOfStateVBoxLayout{};
    };
}

#endif
