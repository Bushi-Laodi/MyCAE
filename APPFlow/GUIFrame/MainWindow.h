/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   MainWindow.h
 * @brief  主窗口界面
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-06-12
 */
#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QMainWindow>
#include "GUIFrameAPI.h"

#include "FITK_Kernel/FITKCore/FITKAbstractGUIObject.h"

#include <SARibbonMainWindow.h>
#include <SARibbonPannelItem.h>

#include <QFont>

class SARibbonBar;
class SARibbonPannel;

namespace Ui {
    class MainWindow;
}

namespace GUI
{
    class MainMenuBase;
    class ControlPanelWidget;
    class RenderWidget;
    class PropertyWidget;
    class GroupPropertyWidget;
    class ActionEventHandler;

    class MainTreeWidget;
    class TabWidget;

    /**
     * @brief  主窗口界面类
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date   2024-06-12
     */
    class GUIFRAMEAPI MainWindow : public SARibbonMainWindow, public Core::FITKAbstractGUIObject
    {
        Q_OBJECT;
    public:
        /**
         * @brief  构造函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        MainWindow(QWidget *parent = nullptr);
        /**
         * @brief  析构函数
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        ~MainWindow();

        /**
         * @brief  获取界面的action消息处理器
         * @return 消息处理器
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @data   2024-06-12
         */
        ActionEventHandler* getActionEventHandle() const;
        /**
         * @brief  获取树结构子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        MainTreeWidget* getTreeWidget() const;
        /**
         * @brief  获取渲染子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        RenderWidget* getRenderWidget() const;
        /**
         * @brief  获取属性子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        PropertyWidget* getPropertyWidget() const;
        /**
         * @brief  获取组属性子部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        GroupPropertyWidget* getGroupPropertyWidget() const;
        /**
         * @brief 创建按钮
         * @param[i]  toolTip        提示信息
         * @param[i]  objectName     按钮名称
         * @param[i]  iconPath       按钮图标
         * @param[i]  iconText       图注
         * @return QAction* action对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-11
         */
        QAction* createAction(const QString &toolTip, const QString &objectName, const QString &iconPath = "", const QString& iconText = "");
    signals:
        ;
        /**
         * @brief    mainWindow关闭信号
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        void sigMainWindowClose();
    protected:
        /**
         * @brief    关闭事件重写
         * @param[i] event    事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-30
         */
        virtual void closeEvent(QCloseEvent *event) override;
    private:
        /**
         * @brief    初始化Ribbon页面
         * @return   void
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2025-06-13
         */
        void initRibbonBar();
        /**
         * @brief  初始化中间部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        void initCentralWidget();
        /**
         * @brief 模块初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-11
         */
        void initApplicationButton();
        void initHome();
        void initGeometry();
        void initMesh();
        void initSetting();
        void initResult();
        void initHelp();
        /**
         * @brief 按钮修改
         * @param[i]  action         按钮对象
         * @param[i]  iconPath       图标路径
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-11
         */
        bool changeAction(QAction* action, const QString iconPath, const QString& iconText = "");
        /**
         * @brief RibbonPannel中添加action
         * @param[i]  pannel         pannel对象
         * @param[i]  action         action对象
         * @param[i]  actionType     action的类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        void pannelAddAction(SARibbonPannel* pannel, QAction* action, SARibbonPannelItem::RowProportion actionType = SARibbonPannelItem::Large);
        /**
         * @brief 查找action
         * @param[i]  actionName     action名称
         * @return QAction* action对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        QAction* getAction(const QString actionName);
    private:
        QWidget* _currentWidget = nullptr;
        /**
         * @brief RibbonBar对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        SARibbonBar* _ribbonBar = nullptr;
        /**
         * @brief 树结构界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        MainTreeWidget* _treeWidget = nullptr;
        /**
         * @brief Tab界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        TabWidget* _tabWidgete = nullptr;
        /**
         * @brief  渲染部件
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        RenderWidget* m_RenderWidget{};
        /**
         * @brief  属性面板
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        PropertyWidget* m_PropertyWidget{};
        /**
         * @brief  组属性面板
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-11
         */
        GroupPropertyWidget* m_GroupPropertyWidget{};
        /**
         * @brief  action点击事件处理
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-06-12
         */
        ActionEventHandler* m_ActionHandler{};
        /**
         * @brief 字符格式
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        QFont _font = QFont("Arial", 9);

    };
}

#endif // !__MAINWINDOW_H__
