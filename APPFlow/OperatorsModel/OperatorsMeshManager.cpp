/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsMeshManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUICalculateDialog/BoundaryWidget.h"
#include "GUIDialog/GUIMeshDialog/MeshInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMesherDriver.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshProcessor.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionGeometryRefine.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"

#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

namespace ModelOper
{
    bool OperatorsMeshManager::execGUI()
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        
        if (_emitter == nullptr)return false;
        QString actionName = _emitter->objectName();
        if (actionName == "actionMesh") {
            // 获取单例
            auto meshGen = Interface::FITKMeshGenInterface::getInstance();
            auto manager = meshGen->getGeometryMeshSizeManager();
            // 网格划分
            auto meshDriver = meshGen->getMesherDriver();
            if (meshDriver == nullptr) return false;

            auto regionManager = meshGen->getRegionMeshSizeMgr();
            auto regionGeoRefManager = meshGen->getRegionGeometryRefineManager();
            bool runSnappy = false;
            if (regionManager && regionGeoRefManager) {
                runSnappy = (!(regionManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom).isEmpty()))
                    || regionGeoRefManager->getDataCount();
            }
            
            //工作路径获取
            QString workDir = "";
            if (FITKAPP->getAppSettings()) {
                workDir = FITKAPP->getAppSettings()->getWorkingDir();
            }
            if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";

            //网格划分路径指定
            QString meshGenDir = workDir + "/case";

            meshDriver->setValue("WorkDir", meshGenDir);
            meshDriver->setValue("HasGeoMeshSize", runSnappy);
            meshDriver->startMesher();
            connect(meshDriver, &Interface::FITKAbstractMesherDriver::mesherFinished, [this,meshDriver] {
                disconnect(meshDriver, &Interface::FITKAbstractMesherDriver::mesherFinished, nullptr, nullptr);
                readMesh();
            });
        }
        else if (actionName == "actionMeshEdit") {
            //网格编辑
            GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
            if (mainWindow == nullptr)return false;
            GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
            if (propertyWidget == nullptr)return false;
            GUI::MeshInfoWidget* widget = new GUI::MeshInfoWidget(this);
            propertyWidget->setWidget(widget);
        }
        else if (actionName == "actionClearMesh") {
            //清除网格数据
            auto globalData = FITKAPP->getGlobalData();
            if (globalData == nullptr)return false;
            Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
            if (meshData == nullptr)return false;
            meshData->clearMesh();

            //清除求解器参数中对应的边界
            auto physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
            if (physicsData) {
                auto boundaryManager = physicsData->getBoundaryManager();
                if (boundaryManager)boundaryManager->clear();
            }
            
            //如果当前界面是求解器边界参数界面，清除界面
            if (_mainWindow == nullptr) return false;
            GUI::BoundaryWidget* boundWidget = dynamic_cast<GUI::BoundaryWidget*>(_mainWindow->getPropertyWidget()->getCurrentWidget());
            if (boundWidget) {
                _mainWindow->getPropertyWidget()->init();
            }

            //刷新
            treeOper->updateTree();
            graphOper->reRender();
        }
        return true;
    }

    bool OperatorsMeshManager::execProfession()
    {
        return true;
    }
    void OperatorsMeshManager::readMesh()
    {
        // 获取单例
        auto meshGen = Interface::FITKMeshGenInterface::getInstance();
        // 读取网格
        auto meshProcessor = meshGen->getMeshProcessor();
        if (meshProcessor == nullptr) return;

        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";

        //网格划分路径指定
        QString meshGenDir = workDir + "/case";

        meshProcessor->setValue("WorkDir", meshGenDir);
        meshProcessor->start();
        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        // 网格对象
        auto mesh = FITKAPP->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        graphOper->updateGraph(mesh->getDataObjectID());
        //更新边界与边界网格的对应关系
        Interface::FITKFlowPhysicsHandlerFactory* factoryData = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (!factoryData) return;
        factoryData->resetBoundaryMesh();

        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        treeOper->updateTree();
    }
}