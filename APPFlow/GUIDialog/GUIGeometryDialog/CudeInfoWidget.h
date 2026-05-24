/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CudeInfoWidget.h
 * @brief cude create or edit widget
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _CudeInfoWidget_H
#define _CudeInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

class QTableWidgetItem;

namespace Ui{
    class CudeInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelBox;
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
     * @brief cude create or edit widget
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIGeometryDialogAPI CudeInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief create constructor
         * @param[i]  oper           operators
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief edit constructor
         * @param[i]  obj            My Param doc
         * @param[i]  oper           My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CudeInfoWidget(Interface::FITKAbsGeoModelBox* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Cude Info Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~CudeInfoWidget();
        /**
         * @brief init
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief Set the Basic Point of cude
         * @param[i]  point          base point
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setBasicPoint(double* point);
        /**
         * @brief Set the Face Group of cude
         * @param[i]  rowIndex       faceGroup index
         * @param[i]  facesId        faceIDS
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
    protected:
        /**
         * @brief close event override
         * @param[i]  event          event object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void closeEvent(QCloseEvent *event) override;
    private slots:
        ;
        /**
         * @brief basePoint button clicled slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_BasicPoint_clicked();
        /**
         * @brief cencel button clicked slot
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
         * @brief check widget value
         * @return true  success
         * @return false  fial
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
         * @brief current type isCreate
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool _isCreate = false;
        /**
         * @brief ui object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::CudeInfoWidget* _ui = nullptr;
        /**
         * @brief face group widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
