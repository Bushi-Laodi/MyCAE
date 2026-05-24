/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   MainMenuBase.h
 * @brief  主界面菜单基类（为ribbon风格预留接口）
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-11
 */
#ifndef __MAINMENUBASE_H__
#define __MAINMENUBASE_H__
#include <QIcon>
#include <QList>
#include <QMenu>
#include "GUIWidgetAPI.h"

namespace GUI
{
    class GUIWIDGETAPI MenuActionItem {
    public:
        MenuActionItem();
        MenuActionItem(QString title, QString action, QIcon icon, bool checkable = false, QString checkGroup = "");
        MenuActionItem(QString title, QString action, bool checkable = false, QString checkGroup = "");
        MenuActionItem(QString title, QString action, QList<MenuActionItem*> childs);
        ~MenuActionItem() = default;

        bool getSeparator() const;
        bool getCheckable() const;
        QString getCheckGroup() const;
        QString getTitle() const;
        QIcon getIcon() const;
        QString getOperAction() const;
        QList<MenuActionItem*> getChilds() const;
        QKeySequence getKey() const;
    private:
        bool m_Separator{ false };
        bool m_Checkable{ false };
        QString m_CheckGroup{};
        QString m_Title{};
        QIcon m_Icon{};
        QString m_OperAction{};
        QList<MenuActionItem*> m_Childs{};
        QKeySequence m_Key{};
    };

    class MainWindow;
    /**
     * @brief  主界面菜单基类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-11
     */
    class GUIWIDGETAPI MainMenuBase : public QObject
    {
    public:
        /**
         * @brief  构造函数
         * @param  mainWindow 主窗口
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        MainMenuBase(MainWindow* mainWindow);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        virtual ~MainMenuBase() = default;
        /**
         * @brief  添加菜单
         * @param  title 菜单标题
         * @param  actions actions列表
         * @param  parent 父级菜单
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        virtual void addMenu(QString title, QList<MenuActionItem*> actions, QMenu* parent = nullptr);
        /**
         * @brief  添加工具条菜单
         * @param  title 菜单标题
         * @param  actions actions列表
         * @param  style 显示风格
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        virtual void addToolMenu(QString title, QList<MenuActionItem*> actions, Qt::ToolButtonStyle style = Qt::ToolButtonStyle::ToolButtonIconOnly);
    protected:
        /**
         * @brief  主窗口
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        MainWindow* m_MainWindow{};

    };
}
#endif // !__MAINMENUBASE_H__
