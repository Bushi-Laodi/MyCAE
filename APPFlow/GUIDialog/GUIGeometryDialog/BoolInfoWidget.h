/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file BoolInfoWidget.h
 * @brief bool模型编辑界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-08
 * 
 */
#ifndef _BoolInfoWidget_H
#define _BoolInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class BoolInfoWidget;
}

namespace Interface {
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;
    /**
     * @brief bool模型编辑界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-08
     */
    class GUIGeometryDialogAPI BoolInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Bool Info Widget object
         * @param[i]  obj            模型对象
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        BoolInfoWidget(Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Bool Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        ~BoolInfoWidget();
        /**
         * @brief 初始化参数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void init();
        /**
         * @brief 设置面组
         * @param[i]  rowIndex       面组索引
         * @param[i]  facesId        面组id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
    protected:
        /**
         * @brief 关闭事件重写
         * @param[i]  event          My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void closeEvent(QCloseEvent *event) override;
    private:
        /**
         * @brief 检查数据
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        bool checkValue();
        /**
         * @brief 设置数据至当前界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setDataToWidget();
    private:
        /**
         * @brief 创建模式标记位
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        bool _isCreate = false;
        /**
         * @brief 模型数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        /*Interface::FITKAbsGeoCommand* _obj = nullptr;*/
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        Ui::BoolInfoWidget* _ui = nullptr;
        /**
         * @brief 面组界面组件对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
