/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CylinderInfoWidget.h
 * @brief cylinder create or edit widget
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _CylinderInfoWidget_H
#define _CylinderInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class CylinderInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelCylinder;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;

    class GUIGeometryDialogAPI CylinderInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief create constructor
         * @param[i]  oper           operators
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief edit construectors
         * @param[i]  obj            geometry objectors
         * @param[i]  oper           operators
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Cylinder Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~CylinderInfoWidget();
        /**
         * @brief init
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief Set the Origin Point object
         * @param[i]  point          My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setOriginPoint(double* point);
        /**
         * @brief Set the Face Group Value object
         * @param[i]  rowIndex       face group index
         * @param[i]  facesId        face ids
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
         * @brief origin points button clicked slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_OriginPoint_clicked();
        /**
         * @brief cancel button clicked slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Cancel_clicked();
        /**
         * @brief create or edit button clicked slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_CreateOrEdit_clicked();
    private:
        /**
         * @brief check value
         * @return true success
         * @return false fail
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
         * @brief current widget isCreate
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool _isCreate = false;
        /**
         * @brief ui object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::CylinderInfoWidget* _ui = nullptr;
        /**
         * @brief face group widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
