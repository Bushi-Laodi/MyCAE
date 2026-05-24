/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GraphModelProvider.h"

// Render VTK
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"

// Graph
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"

// Adaptor
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKViewAdaptorBase.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Data
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"

// Graph widget
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"

namespace GraphData
{
    GraphModelProvider::GraphModelProvider(Comp::FITKGraph3DWindowVTK* graphWidget)
        : GraphProviderBase(graphWidget)
    {

    }

    GraphModelProvider::~GraphModelProvider()
    {
        // 析构三维可视化对象。
        deleteObjsHash(m_modelObjHash);
        deleteObjsHash(m_boundMeshObjHash);
        deleteObjsHash(m_regionMeshObjHash); 
    }

    QString GraphModelProvider::getClassName()
    {
        return "GraphModelProvider";
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> GraphModelProvider::getCurrentGraphObjs()
    {
        // 当前所有模型可视化对象数据。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs;

        // 模型（几何）可视化对象。
        objs << m_modelObjHash.values();

        // 边界网格可视化对象。
        objs << m_boundMeshObjHash.values();

        // 流体域形状可视化对象。
        objs << m_regionMeshObjHash.values();

        return objs;
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphModelProvider::getModelGraphObject(int dataObjId)
    {
        // 模型可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKAbsGeoCommand* model = FITKDATAREPO->getTDataByID<Interface::FITKAbsGeoCommand>(dataObjId);
        if (!model)
        {
            return obj;
        }

        // 模型可视化对象。
        obj = getGraphObject("GeoModelShape", m_modelObjHash, model->getShapeAgent());

        return obj;
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> GraphModelProvider::getAllModelGraphObjects()
    {
        // 模型可视化对象列表。
        // QList<Exchange::FITKFluidVTKGraphObject3D*> objs;

        return m_modelObjHash.values();
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphModelProvider::getBoundMeshGraphObject(int dataObjId)
    {
        // 边界网格可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKBoundaryMeshVTK* boundMesh = FITKDATAREPO->getTDataByID<Interface::FITKBoundaryMeshVTK>(dataObjId);
        if (!boundMesh)
        {
            return obj;
        }

        // 模型可视化对象。
        obj = getGraphObject("BoundMesh", m_boundMeshObjHash, boundMesh);

        return obj;
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> GraphModelProvider::getFuildBoundMeshGraphObjects(int dataObjId)
    {
        // 模型可视化对象。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs;

        // 检查数据ID。
        Interface::FITKUnstructuredFluidMeshVTK* fluidMesh = FITKDATAREPO->getTDataByID<Interface::FITKUnstructuredFluidMeshVTK>(dataObjId);
        if (!fluidMesh)
        {
            return objs;
        }

        // 遍历网格区域。
        int nRegions = fluidMesh->getDataCount();
        for (int i = 0; i < nRegions; i++)
        {
            // 获取边界网格管理器。
            Interface::FITKBoundaryMeshVTKManager* bdMeshMgr = fluidMesh->getBoundaryMeshManager(i);
            if (!bdMeshMgr)
            {
                continue;
            }

            // 生成可视化对象。
            int nBdMesh = bdMeshMgr->getDataCount();

            for (int j = 0; j < nBdMesh; j++)
            {
                Interface::FITKBoundaryMeshVTK* bdMesh = bdMeshMgr->getDataByIndex(j);
                if (!bdMesh)
                {
                    continue;
                }

                int bdMeshId = bdMesh->getDataObjectID();
                Exchange::FITKFluidVTKGraphObject3D* obj = getBoundMeshGraphObject(bdMeshId);
                if (obj)
                {
                    objs.push_back(obj);
                }
            }
        }

        return objs;
    }

    QList<Exchange::FITKFluidVTKGraphObject3D*> GraphModelProvider::getAllMeshGraphObjects()
    {
        return m_boundMeshObjHash.values();
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphModelProvider::getRegionMeshGraphObject(int dataObjId)
    {
        // 流体域形状可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // 检查数据ID。
        Interface::FITKAbstractRegionMeshSize* regionMesh = FITKDATAREPO->getTDataByID<Interface::FITKAbstractRegionMeshSize>(dataObjId);
        if (!regionMesh)
        {
            return obj;
        }

        // 形状可视化对象。
        obj = getGraphObject("RegionMeshPreview", m_regionMeshObjHash, regionMesh);

        return obj;
    }

    Exchange::FITKFluidVTKGraphObject3D* GraphModelProvider::getCurrentGraphObjByDataId(int dataObjId)
    {
        // 查找模型。
        if (m_modelObjHash.contains(dataObjId))
        {
            return m_modelObjHash[dataObjId];
        }

        return nullptr;
    }

    bool GraphModelProvider::updateObjById(int dataObjId, QVariant info)
    {
        // 参数预留。
        Q_UNUSED(info);

        if (!m_modelObjHash.contains(dataObjId))
        {
            return false;
        }

        // 获取可视化对象并更新。
        Core::FITKAbstractGraphObject* obj = m_modelObjHash[dataObjId];
        if (!obj)
        {
            return false;
        }

        obj->update();

        return true;
    }

    void GraphModelProvider::setVertPickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = m_modelObjHash.values();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickVertex);
            }
        }
    }

    void GraphModelProvider::setEdgePickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = m_modelObjHash.values();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickEdge);
            }
        }
    }

    void GraphModelProvider::setFacePickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = m_modelObjHash.values();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickFace);
            }
        }
    }

    void GraphModelProvider::setSolidPickable(int dataObjId)
    {
        // 开启可拾取状态。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = m_modelObjHash.values();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickNone);
            }
            else
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickSolid);
            }
        }
    }

    void GraphModelProvider::setNonePickable(int dataObjId)
    {
        // 关闭可拾取状态。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = m_modelObjHash.values();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            if (obj->getDataId() != dataObjId && dataObjId != -1)
            {
                // 其他数据不取消拾取。
                continue;
            }
            else
            {
                obj->setPickMode(Exchange::FITKFluidVTKCommons::ShapePickMode::PickNone);
            }
        }
    }
}   // namespace GraphData