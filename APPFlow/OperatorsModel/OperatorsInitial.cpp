/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsInitial.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/InitialWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFInitialConditions.h"

namespace ModelOper
{
    OperatorsInitial::OperatorsInitial()
    {

    }

    OperatorsInitial::~OperatorsInitial()
    {

    }

    bool OperatorsInitial::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return false;
        auto initData = phyData->getInitialConditions();
        if (initData == nullptr)return false;

        if (_senderName == "actionInitialEdit") {
            GUI::InitialWidget* widget = new GUI::InitialWidget(this, propertyWidget);
            propertyWidget->setWidget(widget);
        }
        else if(_senderName == "actionInitialDeleteGeo") {
            //几何删除后，求解器节点patch删除对应的patch对象，并更新界面
            int geoId = -1;
            argValue("objID", geoId);
            int pathNum = initData->getPatchCount();
            for (int i = 0; i < pathNum; i++) {
                auto patch = initData->getPatch(i);
                if (patch == nullptr)continue;
                if (geoId == patch->getGeometryId()) {
                    initData->removePatch(i);
                    break;
                }
            }
            GUI::InitialWidget* initWidget = dynamic_cast<GUI::InitialWidget*>(propertyWidget->getCurrentWidget());
            if (initWidget) {
                initWidget->updateWidget();
            }
        }
       
        return true;
    }

    bool OperatorsInitial::execProfession()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return false;
        auto initData = phyData->getInitialConditions();
        if (initData == nullptr)return false;

        if (_senderName == "actionInitialEdit") {

        }
        else if (_senderName == "actionInitialDeleteGeo") {

        }
        return true;
    }
}

