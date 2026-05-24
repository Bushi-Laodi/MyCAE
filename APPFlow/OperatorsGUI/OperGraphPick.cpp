/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperGraphPick.h"

// App
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Graph
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObjectPick.h"

// Render
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphRender.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphObjectVTK.h"

// Pick data provider
#include "GUIWidget/PickedDataProvider.h"

// Graph data manager
#include "GraphDataProvider/GraphProviderManager.h"

// GUI
#include "GUIFrame/MainWindow.h"
#include "GUIFrame/RenderWidget.h"
#include "FITK_Component/FITKWidget/FITKMdiArea.h"
#include "GUIWidget/GUIPickInfo.h"

namespace GUIOper
{
    OperGraphPick::OperGraphPick()
    {
        // 实例化高亮可视化对象。
        m_graphObjPick = new Exchange::FITKFluidVTKGraphObjectPick;
        m_graphObjPick->setVisible(false);

        // 初始化拾取数据管理器。
        m_pickedProvider = GraphData::PickedDataProvider::getInstance();

        // 初始化可视化对象三维数据。
        m_graphObjPick->setPickedData(m_pickedProvider->getPickedDataSet());
    }

    OperGraphPick::~OperGraphPick()
    {
        // 析构。
        if (m_graphObjPick)
        {
            m_graphObjPick->removeFromGraphWidget();

            delete m_graphObjPick;
            m_graphObjPick = nullptr;
        }
    }

    void OperGraphPick::picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor* actor, int index, double* pickedWorldPos)
    {
        if (!graphWindow || !actor)
        {
            return;
        }

        GUI::GUIPickInfo::PickObjType pickType = GUI::GUIPickInfo::GetPickInfo()._pickObjType;
        m_pickedProvider->addPicked(actor, index, pickedWorldPos);

        // 设置演员样式。
        //@{
        switch (pickType)
        {
        case GUI::GUIPickInfo::POBJVert:
            m_graphObjPick->setPickedType(1);
            break;
        case GUI::GUIPickInfo::POBJEdge:
            m_graphObjPick->setPickedType(2);
            break;
        case GUI::GUIPickInfo::POBJFace:
        case GUI::GUIPickInfo::POBJSolid:
            m_graphObjPick->setPickedType(3);
            break;
        default:
            m_graphObjPick->setVisible(false);
            return;
        }
        //@}

        // 添加到渲染窗口。
        m_graphObjPick->setVisible(true);
        addGraphObjectToWidget(m_graphObjPick, graphWindow);

        // 刷新窗口。
        graphWindow->reRender();
    }

    void OperGraphPick::picked(Comp::FITKGraph3DWindowVTK* graphWindow, vtkActor2D* actor)
    {
        if (!graphWindow || !actor)
        {
            return;
        }

        m_pickedProvider->addPicked(actor);

        // 添加到渲染窗口。
        m_graphObjPick->setVisible(true);
        addGraphObjectToWidget(m_graphObjPick, graphWindow);

        // 刷新窗口。
        graphWindow->reRender();
    }

    void OperGraphPick::picked(Comp::FITKGraph3DWindowVTK* graphWindow, QList<vtkActor*> actors, vtkPlanes* cutPlane)
    {
        GUI::GUIPickInfo::PickObjType pickType = GUI::GUIPickInfo::GetPickInfo()._pickObjType;
        m_pickedProvider->addPicked(actors, cutPlane);

        // 如果不是Individually则不执行框选。
        auto pickMethod = GUI::GUIPickInfo::GetPickInfo()._pickMethod;
        if (pickMethod != GUI::GUIPickInfo::PickMethod::PMIndividually)
        {
            return;
        }

        // 设置演员样式。
        //@{
        switch (pickType)
        {
        case GUI::GUIPickInfo::POBJVert:
            m_graphObjPick->setPickedType(1);
            break;
        case GUI::GUIPickInfo::POBJEdge:
            m_graphObjPick->setPickedType(2);
            break;
        case GUI::GUIPickInfo::POBJFace:
        case GUI::GUIPickInfo::POBJSolid:
            m_graphObjPick->setPickedType(3);
            break;
        default:
            m_graphObjPick->setVisible(false);
            return;
        }
        //@}

        // 添加到渲染窗口。
        m_graphObjPick->setVisible(true);
        addGraphObjectToWidget(m_graphObjPick, graphWindow);

        // 刷新窗口。
        graphWindow->reRender();
    }

    void OperGraphPick::clear(Comp::FITKGraph3DWindowVTK* graphWindow)
    {
        // 清空拾取数据。
        GraphData::PickedDataProvider::getInstance()->clearPickedData();

        // 取消高亮并隐藏可视化对象。
        if (m_graphObjPick)
        {
            m_graphObjPick->disHighlight();
            m_graphObjPick->setVisible(false);

            if (!graphWindow)
            {
                graphWindow = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(m_graphObjPick->getGraphWidget());
            }
        }

        // 刷新窗口。
        if (graphWindow)
        {
            graphWindow->reRender();
        }
    }

    void OperGraphPick::setPickedColor(QColor color)
    {
        if (m_graphObjPick)
        {
            m_graphObjPick->setColor(color);
        }
    }

    void OperGraphPick::addGraphObjectToWidget(Exchange::FITKFluidVTKGraphObject3D* obj, Comp::FITKGraph3DWindowVTK* graphWidget)
    {
        if (!obj || !graphWidget)
        {
            return;
        }

        // 添加可视化对象。
        //@{
        obj->removeFromGraphWidget();
        graphWidget->addObject(obj->getRenderLayer(), obj, true);
        //}

        // 刷新窗口。
        //@{
        graphWidget->reRender();
        //@}
    }

    void OperGraphPick::setActorStateByPickInfo(int pickObjType, int pickMethod, int dataObjId)
    {
        GraphData::GraphProviderManager::getInstance()->setActorStateByPickInfo(pickObjType, pickMethod, dataObjId);
    }
}  // namespace GUIOper