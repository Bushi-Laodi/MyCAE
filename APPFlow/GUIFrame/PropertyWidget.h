/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   PropertyWidget.h
 * @brief  属性面板子部件
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __PROPERTYWINDOW_H__
#define __PROPERTYWINDOW_H__

#include "GUIFrameAPI.h"
#include "PanelWidgetBase.h"

namespace GUI
{
    /**
     * @brief  属性面板子部件类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-11
     */
    class GUIFRAMEAPI PropertyWidget : public PanelWidgetBase
    {
        Q_OBJECT
    public:
        /**
         * @brief  构造函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        explicit PropertyWidget(MainWindow *mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        ~PropertyWidget() = default;

        void init();
    };
}
#endif // !__PROPERTYWINDOW_H__
