/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file GeometryBoolWidget.h
 * @brief 布尔模型创建界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GeometryBoolWidget_H
#define _GeometryBoolWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

namespace Ui {
    class GeometryBoolWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief bool计算类型
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    enum class BoolType {
        GeoBoolNone,         ///空
        GeoBoolFause,        ///求和
        GeoBoolCut,          ///求差
        GeoBoolCommon,       ///求交
    };

    /**
     * @brief 布尔模型创建界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIGeometryDialogAPI GeometryBoolWidget : public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Geometry Bool Widget object
         * @param[i]  type           布尔操作类型
         * @param[i]  oper           操作器
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GeometryBoolWidget(BoolType type, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Geometry Bool Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GeometryBoolWidget();
        /**
         * @brief  初始化函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    private slots:
        ;
        /**
         * @brief 几何1选择
         * @param[i]  index          几何索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_comboBox_Body1_activated(int index);
        /**
         * @brief 几何2选择
         * @param[i]  index          几何索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_comboBox_Body2_activated(int index);
        /**
         * @brief 几何1点选按钮事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Body1_clicked();
        /**
         * @brief 几何2点选按钮事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Body2_clicked();
        /**
         * @brief 应用按钮事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Apply_clicked();
    private:
        /**
         * @brief 数据检查
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool checkValue();
    private:
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
        Ui::GeometryBoolWidget* _ui = nullptr;
        /**
         * @brief 布尔操作类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        BoolType _type = BoolType::GeoBoolNone;
    };
}

#endif
