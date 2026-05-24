/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file TurbulenceWidget.h
 * @brief 求解器湍流界面 
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _TurbulenceWidget_H
#define _TurbulenceWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QButtonGroup;

namespace Ui {
    class TurbulenceWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFTurbulenceManager;
}

namespace GUI
{
    /**
     * @brief 求解器湍流界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI TurbulenceWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Turbulence Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Turbulence Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~TurbulenceWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 更新界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void updateSubWidget();
    private:
        /**
         * @brief 通过数据对象初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
    private slots:
        ;
        /**
         * @brief RadioButton按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotRadioButtonClicked();
        void slotComboBoxModelChange(int index);

        void slotComboBoxDeltaChange(int index);

        void on_pushButton_ModelUnfold_clicked();

        void on_pushButton_DeltaUnfold_clicked();
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::TurbulenceWidget* _ui = nullptr;
        /**
         * @brief RadioButton group
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QButtonGroup* _radioGroup = nullptr;
        /**
         * @brief  湍流管理器
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-21
         */
        Interface::FITKOFTurbulenceManager* _turManager = nullptr;
    };
}
#endif
