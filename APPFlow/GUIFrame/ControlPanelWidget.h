/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   ControlPanelWidget.h
 * @brief  控制面板子部件
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __CONTROLPANELWIDGET_H__
#define __CONTROLPANELWIDGET_H__

#include "PanelWidgetBase.h"
 /**
  * @brief GUI命名空间
  * @since 1.0.0
  */
namespace GUI
{
    class MainWindow;
    /**
     * @brief  控制面板子部件类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-11
     */
    class ControlPanelWidget : public PanelWidgetBase
    {
        Q_OBJECT
    public:
        /**
         * @brief  构造函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        ControlPanelWidget(MainWindow *mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        ~ControlPanelWidget();

    };
}
#endif // !__CONTROLPANELWINDOW_H__
