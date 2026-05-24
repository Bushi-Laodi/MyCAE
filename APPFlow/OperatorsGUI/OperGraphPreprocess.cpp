/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperGraphPreprocess.h"

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Graph widget and object
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"
#include "GraphDataProvider/GraphModelProvider.h"
#include "GraphDataProvider/GraphMarkProvider.h"

// GUI
#include "GUIFrame/MainTreeWidget.h"

namespace GUIOper
{
    void OperGraphPreprocess::updateGraph(int dataObjId, bool forceUpdate)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取或创建可视化对象。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = getGraphObjectsByDataId(dataObjId);

        // 添加至三维窗口。
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            obj->update(forceUpdate);

            addGraphObjectToWidget(obj, graphWidget, false);
        }
    }

    void OperGraphPreprocess::updateGraphByType(int type, GraphOperParam param)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取符号可视化对象管理器。
        GraphData::GraphMarkProvider* markProvider = GraphData::GraphProviderManager::getInstance()->getMarkProvider(graphWidget);
        if (!markProvider)
        {
            return;
        }

        // 获取或创建可视化对象。（type数值与树形节点类型枚举对应。）
        Exchange::FITKFluidVTKGraphObject3D* obj = markProvider->getGraphObjectByType(type);
        if (!obj)
        {
            return;
        }

        // 首先添加至三维窗口。
        addGraphObjectToWidget(obj, graphWidget, false);

        // 刷新数据。
        obj->update(param.ForceUpdate);

        // 高亮/取消高亮。
        switch (param.HighlightMode)
        {
        case HighlightLevel::DisHighlight:
        {
            obj->disHighlight();
            break;
        }
        case HighlightLevel::Highlight:
        {
            obj->highlight();
            break;
        }
        case HighlightLevel::AdvHighlight:
        {
            obj->advanceHighlight(Exchange::FITKFluidVTKCommons::ShapeType::ShapeTypeNone, param.AdvHighlightIndice);
            break;
        }
        default:
            break;
        }
        
        // 隐藏或显示。
        obj->setVisible(param.Visibility);

        // 刷新窗口。
        graphWidget->reRender();
    }

    Exchange::FITKFluidVTKGraphObject3D* OperGraphPreprocess::getModelGraphObjectByDataId(int dataObjId)
    {
        // 可视化对象。
        Exchange::FITKFluidVTKGraphObject3D* obj{ nullptr };

        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return obj;
        }

        // 获取模型/网格可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return obj;
        }

        obj = modelProvider->getCurrentGraphObjByDataId(dataObjId);
        return obj;
    }

    void OperGraphPreprocess::setEnableModelTransparent(bool flag)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型/网格可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return;
        }

        // 启用或关闭半透明。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = modelProvider->getAllModelGraphObjects();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (obj)
            {
                obj->setTransparent(flag);
            }
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::setEnableMeshTransparent(bool flag)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型/网格可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return;
        }

        // 启用或关闭半透明。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = modelProvider->getAllMeshGraphObjects();
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (obj)
            {
                obj->setTransparent(flag);
            }
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::setModelVisible(int dataObjId, bool visibility)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型/网格可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return;
        }

        Exchange::FITKFluidVTKGraphObject3D* obj = modelProvider->getModelGraphObject(dataObjId);
        if (obj)
        {
            obj->setVisible(visibility);
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::setMeshVisible(int dataObjId, bool visibility)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型/网格可视化对象管理器。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (!modelProvider)
        {
            return;
        }

        Exchange::FITKFluidVTKGraphObject3D* obj = modelProvider->getBoundMeshGraphObject(dataObjId);
        if (obj)
        {
            obj->setVisible(visibility);
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::highlight(int dataObjId, QVariant info, QColor color)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 全部高亮。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = getGraphObjectsByDataId(dataObjId);
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            obj->highlight(Exchange::FITKFluidVTKCommons::ShapeType::ShapeTypeNone, color);
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::advHighlight(int dataObjId, QVector<int> & indice, QColor color)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 全部高级高亮。
        QList<Exchange::FITKFluidVTKGraphObject3D*> objs = getGraphObjectsByDataId(dataObjId);
        for (Exchange::FITKFluidVTKGraphObject3D* obj : objs)
        {
            if (!obj)
            {
                continue;
            }

            obj->advanceHighlight(Exchange::FITKFluidVTKCommons::ShapeType::ShapeTypeNone, indice, color);
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::clearHighlight()
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        // 获取模型/网格与符号可视化对象管理器，并清除高亮。
        GraphData::GraphModelProvider* modelProvider = GraphData::GraphProviderManager::getInstance()->getModelProvider(graphWidget);
        if (modelProvider)
        {
            modelProvider->clearHighlight();
        }

        GraphData::GraphMarkProvider* markProvider = GraphData::GraphProviderManager::getInstance()->getMarkProvider(graphWidget);
        if (markProvider)
        {
            markProvider->clearHighlight();
        }

        // 刷新窗口。
        graphWidget->reRender();
    }

    void OperGraphPreprocess::reRender(bool fitView)
    {
        // 获取可视化窗口。
        Comp::FITKGraph3DWindowVTK* graphWidget = getGraphWidget();
        if (!graphWidget)
        {
            return;
        }

        if (fitView)
        {
            graphWidget->fitView();
        }
        else
        {
            graphWidget->reRender();
        }
    }
}  // namespace GUIOper
