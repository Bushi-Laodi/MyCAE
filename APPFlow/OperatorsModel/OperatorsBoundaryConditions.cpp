/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsBoundaryConditions.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/BoundaryWidget.h"
#include "GUIDialog/GUICalculateDialog/BoundaryCreateDialog.h"
#include "GUIDialog/GUICalculateDialog/BoundaryDeleteDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"

namespace ModelOper
{
    OperatorsBoundaryConditions::OperatorsBoundaryConditions()
    {

    }

    OperatorsBoundaryConditions::~OperatorsBoundaryConditions()
    {

    }

    bool OperatorsBoundaryConditions::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        auto physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (physicsData == nullptr)return false;
        auto boundaryManager = physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Create: {
            GUI::BoundaryCreateDialog* dialog = new GUI::BoundaryCreateDialog(this);
            dialog->show();
            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            int objID = -1;
            this->argValue<int>("objID", objID);
            Interface::FITKOFBoundary* boundaryObj = boundaryManager->getDataByID(objID);
            GUI::BoundaryWidget* widget = new GUI::BoundaryWidget(boundaryObj, this, propertyWidget);
            propertyWidget->setWidget(widget);
            break;
        }
        case ModelOper::OperManagerBase::Delete: {
            int objID = -1;
            this->argValue<int>("objID", objID);
            Interface::FITKOFBoundary* boundaryObj = boundaryManager->getDataByID(objID);
            GUI::BoundaryDeleteDialog* dialog = new GUI::BoundaryDeleteDialog(boundaryObj, this);
            dialog->show();
        }
        }

        return false;
    }

    bool OperatorsBoundaryConditions::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Create:{
            treeOper->updateTree();
            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            break;
        }
        case ModelOper::OperManagerBase::Delete: {
            treeOper->updateTree();
            break;
        }
        }

        return true;
    }
}

