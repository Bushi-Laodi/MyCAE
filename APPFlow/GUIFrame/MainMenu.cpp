/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MainMenu.h"
#include "MainWindow.h"
#include <QMenu>
#include "ActionEventHandler.h"

namespace GUI
{
    MainMenu::MainMenu(MainWindow *mainWindow) : MainMenuBase(mainWindow)
    {
    }

    void MainMenu::addMenu(QString title, QList<MenuActionItem*> actions, QMenu * parent)
    {
        if (m_MenuBar == nullptr) {
            m_MenuBar = new QMenuBar;
            m_MainWindow->setMenuBar(m_MenuBar);
        }
        // 如果父级菜单为空，则添加顶级菜单
        if (parent == nullptr) {
            parent = new QMenu();
            parent->setTitle(title);
            m_MenuBar->addMenu(parent);
        }
        // 遍历actions依次添加
        for (auto actionItem : actions) {
            if (actionItem->getSeparator()) { parent->addSeparator(); continue; }
            // 如果有子级action，则需要添加menu
            if (actionItem->getChilds().size() > 0) {
                auto menu = new QMenu();
                menu->setTitle(actionItem->getTitle());
                addMenu(actionItem->getTitle(), actionItem->getChilds(), menu);
                parent->addMenu(menu);
                continue;
            }
            // 否则，只需要添加action
            auto action = parent->addAction(actionItem->getIcon(), actionItem->getTitle());
            action->setObjectName(actionItem->getOperAction());
            connect(action, SIGNAL(triggered()), m_MainWindow->getActionEventHandle(), SLOT(execOperator()));
            // 设置action的可选中状态，并进行分组实现只能单选
            action->setCheckable(actionItem->getCheckable());
            auto checkGroup = actionItem->getCheckGroup();
            if (!checkGroup.isEmpty()) {
                if (!m_ActionGroups.contains(checkGroup))
                    m_ActionGroups.insert(checkGroup, new QActionGroup(parent));
                m_ActionGroups[checkGroup]->addAction(action);
            }
            // 设置action的快捷键
            if (!actionItem->getKey().isEmpty())
                action->setShortcut(actionItem->getKey());
        }
    }

    void MainMenu::addToolMenu(QString title, QList<MenuActionItem*> actions, Qt::ToolButtonStyle style)
    {
        if (m_MenuToolBars.contains(title)) m_MenuToolBars.remove(title);

        m_MenuToolBars[title] = new QToolBar;
        m_MenuToolBars[title]->setToolButtonStyle(style);

        // 遍历actions依次添加
        for (auto actionItem : actions) {

            if (actionItem->getSeparator()) {
                m_MenuToolBars[title]->addSeparator();
                continue;
            }
            auto action = m_MenuToolBars[title]->addAction(actionItem->getIcon(), actionItem->getTitle());
            action->setObjectName(actionItem->getOperAction());
            connect(action, SIGNAL(triggered()), m_MainWindow->getActionEventHandle(), SLOT(execOperator()));
        }
        m_MainWindow->addToolBar(Qt::TopToolBarArea, m_MenuToolBars[title]);

    }

}
