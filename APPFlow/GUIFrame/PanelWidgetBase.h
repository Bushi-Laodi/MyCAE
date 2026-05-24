/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   PanelWidgetBase.h
 * @brief  主窗口子面板子类
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __PANELWIDGETBASE_H__
#define __PANELWIDGETBASE_H__

#include "GUIFrameAPI.h"
#include <QWidget>
#include <QList>

class QLabel;
class QVBoxLayout;

namespace GUI
{
    class MainWindow;
    /**
     * @brief  主窗口子面板子类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-11
     */
    class GUIFRAMEAPI PanelWidgetBase : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * @brief  构造函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        explicit PanelWidgetBase(MainWindow *mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        virtual ~PanelWidgetBase() = 0;
        /**
         * @brief  设置子面板标题
         * @param  title 标题
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        void setTitle(QString title);
        /**
         * @brief  获取子面板标题
         * @return 子面板标题
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        QString getTitle() const;
        /**
         * @brief  设置面板窗口标题的高度(默认值50)
         * @param  height 高度值
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        void setTitleHeight(int height = 50);
        /**
         * @brief  设置子部件的显示部件
         * @param  widget 显示部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        void setWidget(QWidget *widget);
        /**
         * @brief  移除显示的子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        void removeWidget();
        /**
         * @brief  获取当前界面
         * @param  widget 显示部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        QWidget* getCurrentWidget();
    protected:
        /**
         * @brief  主窗口对象
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        MainWindow *m_MainWindow{};
        /**
         * @brief  面板窗口的标题Label
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        QLabel *m_TitleLabel{};
        /**
         * @brief  面板窗口的布局
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        QVBoxLayout *m_Layout{};
        /**
         * @brief  面板窗口显示的内容部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        QWidget* m_Widget{};
    };
}
#endif // !__PANELWIDGETBASE_H__
