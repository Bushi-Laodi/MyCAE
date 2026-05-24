/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

//#define OCCRENDERMODE
#include <QVariant>
#include <QStackedLayout>
#include <QPushButton>
#include "RenderWidget.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QMdiSubWindow>
#include "FITK_Component/FITKWidget/FITKMdiArea.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponentFactory.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponentInterface.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowInterface.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
//#include "FITK_Component/FITKRenderWindowOCC/FITKGraph3DWindowOCCInterface.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace GUI
{
    RenderWidget::RenderWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
    {
        this->setTitle(tr("Graphic Display"));

        m_MdiArea = new Comp::FITKMdiArea(this);

        // 如果当前没有FITK应用程序实例，则不执行后续操作
        if (!FITKAPP)
            return;


#ifdef OCCRENDERMODE
        // 尝试获取名为"Graph3DWindow"的三维渲染组件
        AppFrame::FITKComponentInterface *graph3DCompBase =
            FITKAPP->getComponents()->getComponentByName("Graph3DWindowOCC");
        auto graph3DComp = dynamic_cast<Render::FITKGraph3DWindowOCCInterface *>(graph3DCompBase);
        // 如果获取失败，则不执行后续操作
        if (nullptr == graph3DComp)
            return;

        // 获取三维渲染组件的界面，并将其作为一个子窗口添加到MDI区域
        QWidget *graph3DWidget = graph3DComp->getWidget(1);
        auto id = m_MdiArea->addSubWidget(graph3DWidget, "Graph3DWindowOCC");
#else
        // 尝试获取名为"Graph3DWindow"的三维渲染组件
        AppFrame::FITKComponentInterface *graph3DCompBase =
            FITKAPP->getComponents()->getComponentByName("Graph3DWindowVTK");
        auto graph3DComp = dynamic_cast<Comp::FITKGraph3DWindowInterface *>(graph3DCompBase);
        // 如果获取失败，则不执行后续操作
        if (nullptr == graph3DComp)
            return;

        // 获取三维渲染组件的界面，并将其作为一个子窗口添加到MDI区域
        QWidget *graph3DWidget = graph3DComp->getWidget(1);
        Comp::FITKGraph3DWindowVTK* graph3DVTKWidget = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(graph3DWidget);
        if (graph3DVTKWidget != nullptr) {
            graph3DVTKWidget->SetGlobalWarningDisplay(false);
            graph3DVTKWidget->setIsShowActions(true);
        }
        auto id = m_MdiArea->addSubWidget(graph3DWidget, "");
#endif // OCCRENDERMODE


        this->setWidget(m_MdiArea);

        m_MdiArea->setLayoutType(Comp::FITKVportsLayoutType::CurrentMax);
        // 最大化显示
        /*auto currentSubWidget = m_MdiArea->getSubWidget(id);
        if (currentSubWidget != nullptr)
        {
            currentSubWidget->showMaximized();
        }*/

    }

    Comp::FITKMdiArea * RenderWidget::getMdiArea() const
    {
        return m_MdiArea;
    }

}
