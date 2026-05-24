/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   GroupPropertyWidget.h
 * @brief  图形（组）属性面板
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __GROUPPROPERTYWIDGET_H__
#define __GROUPPROPERTYWIDGET_H__

#include "PanelWidgetBase.h"

namespace GUI
{
    class MainWindow;
    /**
     * @brief  图形（组）属性面板类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-11
     */
    class GroupPropertyWidget : public PanelWidgetBase
    {
        Q_OBJECT
    public:
        /**
         * @brief 构造函数
         * @param parent 父窗口对象
         * @since 1.0.0
         */
        GroupPropertyWidget(MainWindow *mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        ~GroupPropertyWidget();

    };
}

#endif // !__GROUPPROPERTYWIDGET_H__
