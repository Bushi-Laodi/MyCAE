/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   RunWidget.h
 * @brief  求解器运行界面
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-09-02
 *********************************************************************/
#ifndef _RunWidget_H
#define _RunWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QAbstractButton;

namespace Ui {
    class RunWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFRunControl;
}

namespace GUI
{
    /**
     * @brief  求解器CPU状态
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-09-02
     */
    enum class RunCPUType {
        Serial,
        Parallel,
    };

    /**
     * @brief  求解器运行界面
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-09-02
     */
    class GUICalculateDialogAPI RunWidget : public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper      操作器对象
         * @param[i] parent    父对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        ~RunWidget();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void init();
        /**
         * @brief    显示事件
         * @param[i] event   事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void showEvent(QShowEvent * event);
        /**
         * @brief    隐藏事件
         * @param[i] event   事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void resizeEvent(QResizeEvent * event);
    private slots:
        ;
        /**
         * @brief    CPU状态修改事件
         * @param[i] button    QRadioButton对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void slotCPUChange(QAbstractButton* button);
        /**
         * @brief    进程结束事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void slotProcessFinish();
        /**
         * @brief    CPU核数修改事件
         * @param[i] arg1 CPU核数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void on_spinBox_NumOfPro_valueChanged(int arg1);
        /**
         * @brief    停止按钮点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void on_pushButton_Stop_clicked();
        /**
         * @brief    运行按钮点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void on_pushButton_Run_clicked();
    private:
        /**
         * @brief    更新时间参数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void updataTime();
        /**
         * @brief    更新输出参数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void updateOutput();
        /**
         * @brief    初始化CPU
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void initCPU();
        /**
         * @brief    更新CPU
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void updateCPU();
        /**
         * @brief    清理算例文件
         * @param[i] casePath  算例路径 
         * @return   bool      是否成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-29
         */
        bool clearCasePath(QString casePath);
        /**
         * @brief    写出算例
         * @param[i] casePath  算例路径 
         * @return   bool      是否成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-29
         */
        bool writeCase(QString casePath);
        /**
         * @brief    创建启动脚本
         * @param[i] workDir   工作路径
         * @param[i] caseDir   算例路劲
         * @return   QString   脚本路径
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        QString creatStartSh(QString workDir, QString caseDir);
        /**
         * @brief    设置是否是运行状态
         * @param[i] isRun    是否是运行状态
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-02
         */
        void setRunType(bool isRun);
        /**
         * @brief    获取特定区域处理命令
         * @return   QStringList
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-07-07
         */
        QStringList getRegionTouch(QString caseDir);

    private:
        /**
         * @brief  UI对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Ui::RunWidget* _ui = nullptr;
        /**
         * @brief  求解器运行参数对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-02
         */
        Interface::FITKOFRunControl* _runConObj = nullptr;
    };
}

#endif
