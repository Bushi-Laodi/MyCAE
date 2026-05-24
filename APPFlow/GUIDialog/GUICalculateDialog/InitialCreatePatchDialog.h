/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file InitialCreatePatchDialog.h
 * @brief initial创建patch对话框
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _InitialCreatePatchDialog_H
#define _InitialCreatePatchDialog_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIDialogBase.h"

namespace Ui {
    class InitialCreatePatchDialog;
}

namespace Interface {
    class FITKOFPhysicsData;
    class FITKFlowPhysicsHandlerFactory;
}

class QButtonGroup;
class QCheckBox;

namespace GUI
{
    class InitialWidget;
    /**
     * @brief initial创建patch对话框
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI InitialCreatePatchDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new InitialCreatePatchDialog object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        InitialCreatePatchDialog(QWidget* parent);
        /**
         * @brief Destroy the InitialCreatePatchDialog object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~InitialCreatePatchDialog();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    private slots:
        ;
        /**
         * @brief    取消按钮点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-04
         */
        void on_pushButton_Cancel_clicked();
        /**
         * @brief    确定按钮点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-04
         */
        void on_pushButton_OK_clicked();
    private:
        /**
         * @brief  父界面
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        InitialWidget* _parentWidget = nullptr;
        /**
         * @brief  ui
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        Ui::InitialCreatePatchDialog* _ui = nullptr;
        /**
         * @brief  场列表
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        QList<QCheckBox*> _fields = {};
        /**
         * @brief  radio组对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        QButtonGroup* _radioGroup = nullptr;
        /**
         * @brief  实际数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        Interface::FITKOFPhysicsData* _physicsData = nullptr;
        /**
         * @brief  工厂数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-04
         */
        Interface::FITKFlowPhysicsHandlerFactory* _factoryData = nullptr;
    };
}

#endif
