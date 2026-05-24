/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GraphMarkProvider.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"

// Graph
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObjectMaterialPoints.h"

// Adaptor
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKViewAdaptorBase.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Data
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKZonePoints.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

// GUI
#include "GUIFrame/MainTreeWidget.h"

namespace GraphData
{
    GraphMarkProvider::GraphMarkProvider(Comp::FITKGraph3DWindowVTK* graphWidget)
        : GraphProviderBase(graphWidget)
    {

    }

    GraphMarkProvider::~GraphMarkProvider()
    {
        // 析构三维可视化对象。
        deleteObjsHash(m_tempTypeObjHash);
    }

    QString GraphMarkProvider::getClassName()
    {
        return "GraphMarkProvider";
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> GraphMarkProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs;

        // 临时标识符可视化对象。
        // objs << m_tempTypeObjHash.values();

        return objs;
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphMarkProvider::getGraphObjectByType(int type)
    {
        // 符号可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // type数值与树形节点类型枚举对应。
        switch (type)
        {
        // 材料点。
        case int(GUI::MainTreeEnum::MainTree_MeshPoint):
        {
            // 获取材料点可视化对象。
            obj = getMaterialPointsGraphObject();
            break;
        }
        default:
            break;
        }

        return obj;
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphMarkProvider::getMaterialPointsGraphObject()
    {
        // 全部材料点可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // 获取材料点管理器。
        Interface::FITKMeshGenInterface* interMeshGen = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* mPtsMgr = interMeshGen->getZonePointManager();
        if (!mPtsMgr)
        {
            return obj;
        }

        // 材料点可视化对象。
        obj = getGraphObject("MaterialPoints", m_tempTypeObjHash, mPtsMgr);

        return obj;
    }
}   // namespace GraphData