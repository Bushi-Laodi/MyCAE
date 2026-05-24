/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   PropertyChildWidgetBase.h
 * @brief  属性面板子部件的基类
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-12
 */
#ifndef __PROPERTYCHILDWIDGETBASE_H__
#define __PROPERTYCHILDWIDGETBASE_H__

#include <QWidget>
#include "GUIWidgetAPI.h"

namespace GUI
{
    class MainWindow;
    /**
     * @brief  属性面板子部件的基类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-12
     */
    class GUIWIDGETAPI PropertyChildWidgetBase : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief  构造函数
         * @param  mainWindow 主窗口
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        PropertyChildWidgetBase(MainWindow* mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        virtual ~PropertyChildWidgetBase();

    protected:
        /**
         * @brief  主窗口
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        MainWindow* m_MainWindow{};
    };
}

#endif // !__PROPERTYCHILDWIDGETBASE_H__

