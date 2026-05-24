/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _PostWidget_H
#define _PostWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"
#include <QProcess>

namespace Ui {
    class PostWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    enum class PostExportType {
        Post_VTK,
    };

    class GUICalculateDialogAPI PostWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        PostWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~PostWidget();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        void init();
    private slots:
        ;
        /**
         * @brief    ParaView打开按钮
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void on_pushButton_ParaView_clicked();
        /**
         * @brief    自研后处理点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        void on_pushButton_Post_clicked();
        /**
         * @brief    文件导出按钮点击事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        void on_pushButton_Export_clicked();
    private:
        /**
         * @brief    创建启paraView可以启动openFOAM的结果文件
         * @return   QString     创建的文件路劲
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        QString creatStartParaViewFile();
    private:
        /**
         * @brief  操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief  UI对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        Ui::PostWidget* _ui = nullptr;
        /**
         * @brief  工作路径
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        QString _workDir = "";
        /**
         * @brief  算例路径
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        QString _caseDir = "";
    };
}
#endif
