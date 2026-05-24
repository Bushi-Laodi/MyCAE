/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperGraphEvent3D.h"

// App
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Graph
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"
#include "GraphDataProvider/GraphModelProvider.h"
#include "GraphDataProvider/GraphMarkProvider.h"

// Data
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKZonePoints.h"

// GUI
#include "GUIFrame/MainWindow.h"
#include "GUIFrame/RenderWidget.h"
#include "FITK_Component/FITKWidget/FITKMdiArea.h"

namespace GUIOper
{
    Comp::FITKGraph3DWindowVTK* OperGraphEvent3D::getGraphWidget()
    {
        // 获取主窗口。
        GUI::MainWindow* mainWindow = FITKAPP->getGlobalData()->getMainWindowT<GUI::MainWindow>();
        if (!mainWindow)
        {
            return nullptr;
        }

        // 获取可视化区。
        GUI::RenderWidget* renderWidget = mainWindow->getRenderWidget();
        if (!renderWidget)
        {
            return nullptr;
        }

        Comp::FITKMdiArea* mdiArea = renderWidget->getMdiArea();
        if (!mdiArea)
        {
            return nullptr;
        }

        // 获取当前窗口，尝试转换为三维窗口。
        QWidget* w = mdiArea->getCurrentWidget();
        Comp::FITKGraph3DWindowVTK* graphWidget = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(w);

        return graphWidget;
    }

    void OperGraphEvent3D::addGraphObjectToWidget(Exchange::FITKFluidVTKGraphObject3D* obj, Comp::FITKGraph3DWindowVTK* graphWidget, bool fitView)
    {
        if (!obj || !graphWidget)
        {
            return;
        }

        // 添加可视化对象。
        //@{
        obj->removeFromGraphWidget();
        graphWidget->addObject(obj->getRenderLayer(), obj, true);
        obj->setFITKRender(graphWidget->getRenderer(obj->getRenderLayer()));
        //}

        // 添加附加可视化对象。
        //@{
        QList<Exchange::FITKFluidVTKGraphObject3D*> addinObjs = obj->getAddinGraphObjs();
        for (Exchange::FITKFluidVTKGraphObject3D* addinObj : addinObjs)
        {
            if (!addinObj)
            {
                continue;
            }

            addinObj->removeFromGraphWidget();
            graphWidget->addObject(addinObj->getRenderLayer(), addinObj, true);
            addinObj->setFITKRender(graphWidget->getRenderer(addinObj->getRenderLayer()));
        }
        //}

        // 刷新窗口。
        //@{
        if (fitView)
        {
            graphWidget->fitView();
        }
        else
        {
            graphWidget->reRender();
        }
        //@}
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> OperGraphEvent3D::getGraphObjectsByDataId(int dataObjId)
    {
        // 获取或创建可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs;
        bool isValid = false;

        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return objs;
        }

        // 获取模型与符号可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        GraphData::GraphMarkProvider* markProvider = GraphData::GraphProviderManager::getInstance()->getMarkProvider(graphWidget);
        if (!modelProvider || !markProvider)
        {
            return objs;
        }

        Core::FITKAbstractDataObject* dataObj = FITKDATAREPO->getDataByID(dataObjId);
        if (!dataObj)
        {
            return objs;
        }

        // 检查数据ID是否为模型。
        Interface::FITKAbsGeoCommand* model = dynamic_cast<Interface::FITKAbsGeoCommand*>(dataObj);
        if (model && !isValid)
        {
            obj = modelProvider->getModelGraphObject(dataObjId);
            isValid = true;
        }

        // 检查数据ID是否为流体网格。
        Interface::FITKUnstructuredFluidMeshVTK* fluidMesh = dynamic_cast<Interface::FITKUnstructuredFluidMeshVTK*>(dataObj);
        if (fluidMesh && !isValid)
        {
            objs = modelProvider->getFuildBoundMeshGraphObjects(dataObjId);
            isValid = true;
        }

        // 检查数据ID是否为边界网格。
        Interface::FITKBoundaryMeshVTK* boundMesh = dynamic_cast<Interface::FITKBoundaryMeshVTK*>(dataObj);
        if (boundMesh && !isValid)
        {
            obj = modelProvider->getBoundMeshGraphObject(dataObjId);
            isValid = true;
        }     

        // 检查数据ID是否为流体域形状数据。
        Interface::FITKAbstractRegionMeshSize* regionMesh = dynamic_cast<Interface::FITKAbstractRegionMeshSize*>(dataObj);
        if (regionMesh && !isValid)
        {
            obj = modelProvider->getRegionMeshGraphObject(dataObjId);
            isValid = true;
        }

        // 检查数据ID是否为材料点数据。
        Interface::FITKZonePointManager* matPtsMgr = dynamic_cast<Interface::FITKZonePointManager*>(dataObj);
        Interface::FITKZonePoint* matPt = dynamic_cast<Interface::FITKZonePoint*>(dataObj);
        if ((matPtsMgr || matPt) && !isValid)
        {
            obj = markProvider->getMaterialPointsGraphObject();
            isValid = true;
        }

        if (obj)
        {
            objs.push_back(obj);
        }

        return objs;
    }
}  // namespace GUIOper