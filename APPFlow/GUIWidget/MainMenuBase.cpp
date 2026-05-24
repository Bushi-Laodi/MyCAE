/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MainMenuBase.h"
namespace GUI {

    MenuActionItem::MenuActionItem()
    {
        m_Separator = true;
    }
    MenuActionItem::MenuActionItem(QString title, QString action, QIcon icon, bool checkable, QString checkGroup)
        : m_Title(title), m_OperAction(action), m_Icon(icon), m_Checkable(checkable), m_CheckGroup(checkGroup)
    {
    }

    MenuActionItem::MenuActionItem(QString title, QString action, bool checkable, QString checkGroup)
        : m_Title(title), m_OperAction(action), m_Checkable(checkable), m_CheckGroup(checkGroup)
    {
    }
    MenuActionItem::MenuActionItem(QString title, QString action, QList<MenuActionItem*> childs)
        : m_Title(title), m_OperAction(action), m_Childs(childs)
    {
    }
    bool MenuActionItem::getSeparator() const
    {
        return m_Separator;
    }

    bool MenuActionItem::getCheckable() const
    {
        return m_Checkable;
    }
    QString MenuActionItem::getCheckGroup() const {
        return m_CheckGroup;
    }
    QString MenuActionItem::getTitle() const
    {
        return m_Title;
    }
    QIcon MenuActionItem::getIcon() const
    {
        return m_Icon;
    }
    QString MenuActionItem::getOperAction() const
    {
        return m_OperAction;
    }
    QList<MenuActionItem*> MenuActionItem::getChilds() const
    {
        return m_Childs;
    }

    QKeySequence MenuActionItem::getKey() const
    {
        return m_Key;
    }

    MainMenuBase::MainMenuBase(MainWindow * mainWindow) : m_MainWindow(mainWindow)
    {

    }
    void MainMenuBase::addMenu(QString title, QList<MenuActionItem*> actions, QMenu * parent)
    {
    }

    void MainMenuBase::addToolMenu(QString title, QList<MenuActionItem*> actions, Qt::ToolButtonStyle style) {}
}

