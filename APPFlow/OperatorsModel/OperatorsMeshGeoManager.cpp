/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsMeshGeoManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIMeshDialog/MeshGeoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsMeshGeoManager::OperatorsMeshGeoManager()
    {

    }

    OperatorsMeshGeoManager::~OperatorsMeshGeoManager()
    {

    }

    bool OperatorsMeshGeoManager::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        if (_senderName == "actionMeshGeoEdit") {
            GUI::MeshGeoWidget* widget = new GUI::MeshGeoWidget(this);
            propertyWidget->setWidget(widget);
        }
        else if(_senderName == "actionMeshGeoDelete")
        {
            GUI::MeshGeoWidget* widget = dynamic_cast<GUI::MeshGeoWidget*>(propertyWidget);
            if (widget) {
                widget->updateWidget();
            }
        }
        
        return true;
    }

    bool OperatorsMeshGeoManager::execProfession()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        if (_senderName == "actionMeshGeoEdit") {

        }
        else if (_senderName == "actionMeshGeoDelete")
        {

        }
        return true;
    }
}
