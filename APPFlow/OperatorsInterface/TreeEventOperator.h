/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   TreeEventOperator.h
 * @brief  树形菜单点击事件接口
 */
#ifndef __TREEEVENT_OPERATOR_H_
#define __TREEEVENT_OPERATOR_H_

#include "FITK_Kernel/FITKCore/FITKAbstractOperator.h"
#include "OperatorsInterfaceAPI.h"
#include "GUIFrame/MainWindow.h"
#include <QHash>
#include <QString>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMenu>


namespace EventOper
{
    /**
     * @brief  树形菜单点击事件接口类
     */
    class OperatorsInterfaceAPI TreeEventOperator : public Core::FITKAbstractOperator
    {
        Q_OBJECT

    public:
        explicit TreeEventOperator();
        virtual ~TreeEventOperator() = 0;
        /**
         * @brief  更新树
         */
        virtual void updateTree();

        virtual void moveProcessToStep(int step, void *addInfo = nullptr);
    protected:
        /**
         * @brief  主窗口
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @date   2024-03-27
         */
        GUI::MainWindow* _mainWindow{};
    };

}  // namespace EventOper

#endif