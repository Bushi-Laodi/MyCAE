/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file SphereInfoWidget.h
 * @brief 球体创建、边界界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _SphereInfoWidget_H
#define _SphereInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

namespace Ui {
    class SphereInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelSphere;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;
    /**
     * @brief 球体创建、边界界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIGeometryDialogAPI SphereInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief 创建构造函数
         * @param[i]  oper           操作器
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief 编辑构造函数
         * @param[i]  obj            几何对象
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        SphereInfoWidget(Interface::FITKAbsGeoModelSphere* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Sphere Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~SphereInfoWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 重新设置基点
         * @param[i]  point          点
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setCenterPoint(double* point);
        /**
         * @brief 设置面组
         * @param[i]  rowIndex       面组索引
         * @param[i]  facesId        面组id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
    protected:
        /**
         * @brief 关闭事件重写
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event) override;
    private slots:
        ;
        /**
         * @brief 中心点按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_CenterPoint_clicked();
        /**
         * @brief 取消按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Cancel_clicked();
        /**
         * @brief 创建/编辑按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_CreateOrEdit_clicked();
    private:
        /**
         * @brief 数据检查
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool checkValue();
        /**
         * @brief Set the Data To Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
        /**
         * @brief Get the Data Form Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void getDataFormWidget();
        /**
         * @brief 是否切换为创建模式
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void switchCreateModel(bool isCreate);
    private:
        /**
         * @brief 是否为创建模式
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool _isCreate = false;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::SphereInfoWidget* _ui = nullptr;
        /**
         * @brief 面组界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
