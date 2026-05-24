/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsMeshLocalManager.h"

#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUIMeshDialog/LocalSelectGroupWidget.h"
#include "GUIDialog/GUIMeshDialog/LocalGroupInfoWidget.h"
#include "GUIDialog/GUIMeshDialog/LocalGroupDeleteDialog.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"

#include <QDialog>

namespace ModelOper 
{
    OperatorsMeshLocalManager::OperatorsMeshLocalManager()
    {

    }

    OperatorsMeshLocalManager::~OperatorsMeshLocalManager()
    {

    }

    bool OperatorsMeshLocalManager::execGUI()
    {
        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        Interface::FITKMeshGenInterface* genInterface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKGeometryMeshSizeManager* manger = genInterface->getGeometryMeshSizeManager();
        if (manger == nullptr)return false;

        QString sendName = "";
        int objID = -1;
        argValue("sender", sendName);
        argValue("objID", objID);

        switch (_operType){
        case ModelOper::OperManagerBase::Edit: {
            widget = new GUI::LocalGroupInfoWidget(manger->getDataByID(objID), this);
            break;
        }
        case ModelOper::OperManagerBase::Delete: {
            dialog = new GUI::LocalGroupDeleteDialog(manger->getDataByID(objID), this);
            break;
        }
        case ModelOper::OperManagerBase::Select:widget = new GUI::LocalSelectGroupWidget(this); break;
        }
        
        if (mainWindow->getPropertyWidget() && widget) {
            propertyWidget->setWidget(widget);    
        }

        if (dialog) {
            dialog->show();
        }

        return false;
    }

    bool OperatorsMeshLocalManager::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Edit: {
            propertyWidget->init();
            break;
        }
        case ModelOper::OperManagerBase::Delete: {
            treeOper->updateTree();
            break;
        }
        case ModelOper::OperManagerBase::Select:treeOper->updateTree();break;
        }

        return true;
    }

    void OperatorsMeshLocalManager::eventProcess(int index, QVariant value)
    {

    }
}