/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "InitialWidget.h"
#include "ui_InitialWidget.h"
#include "InitialCreatePatchDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetVBox.h"
#include "FITK_Kernel/FITKEasyParam/FITKParamString.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFInitialConditions.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Kernel/FITKEasyParam/FITKParamString.h"
#include "FITK_Kernel/FITKCore/FITKEnumTransformer.hpp"

#include <QLabel>

namespace GUI
{
    InitialWidget::InitialWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::InitialWidget();
        _ui->setupUi(this);

        init();
    }

    InitialWidget::~InitialWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }
    void InitialWidget::init()
    {
        if (_physicsData == nullptr)return;
        _initValue = _physicsData->getInitialConditions();
        updateWidget();
    }

    void InitialWidget::updateWidget()
    {
        if (_initValue == nullptr) return;
        if (_initValue->getInitPropRegions())
        {
            //初始化区域属性界面
            _ui->tabWidget->removeTab(1);
            _ui->tabWidget->removeTab(0);
            updateRegions();
        }
        else
        {
            //初始化条件数据界面
            _ui->tabWidget->removeTab(2);
            updateBasicWidget();
            updatePatchWidget();
        }
    }

    void InitialWidget::on_pushButton_PatchAdd_clicked()
    {
        InitialCreatePatchDialog dialog(this);
        dialog.exec();
        updatePatchWidget();
    }

    void InitialWidget::updateBasicWidget()
    {
        if (_initValue == nullptr)return;
        auto basicValue = _initValue->getBasicData();
        if (basicValue == nullptr)return;

        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_Basic->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

        for (auto v : basicValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = new Core::FITKWidgetComLine(v, this);
            _ui->verticalLayout_Basic->addWidget(widget);
        }
    }

    void InitialWidget::updatePatchWidget()
    {
        if (_initValue == nullptr)return;

        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_Patch->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

        int patchNum = _initValue->getPatchCount();
        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        for (int i = 0; i < patchNum; i++) {
            auto pathData = _initValue->getPatch(i);
            if (pathData == nullptr)continue;
            if (pathData->getFieldPara() == nullptr)continue;
            if (pathData->getGeometryModel() == nullptr)continue;
            Interface::FITKOFSolverInitialEnum::FITKPatchApplyObjectType type = pathData->getAppltTo();
            QList<QWidget*> widgets = {};
            QString applyType = "";
            switch (type){
            case Interface::FITKOFSolverInitialEnum::Cells:applyType = "Cells"; break;
            case Interface::FITKOFSolverInitialEnum::Faces:applyType = "Faces"; break;
            case Interface::FITKOFSolverInitialEnum::Both:applyType = "Both"; break;
            }
            Core::FITKParamString titleData;
            titleData.setDataObjectName("Apply To");
            titleData.setValue(applyType);
            Core::FITKWidgetComLine* titleWidget = new Core::FITKWidgetComLine(&titleData, this);
            titleWidget->setEnabled(false);
            widgets.append(titleWidget);
            for (auto para : pathData->getFieldPara()->getParameter()) {
                if (para == nullptr)continue;
                Core::FITKWidgetComLine* w = new Core::FITKWidgetComLine(para, this);
                widgets.append(w);
            }
            QPushButton* removeButton = new QPushButton(this);
            removeButton->setText("Remove");
            removeButton->setProperty("PatchIndex", i);
            connect(removeButton, &QPushButton::clicked, [=]() {
                _initValue->removePatch(i);
                updatePatchWidget();
            });
            widgets.append(removeButton);
            Core::FITKWidgetVBox* widget = new Core::FITKWidgetVBox(widgets, this);
            tabWidget->addTab(widget, pathData->getGeometryModel()->getDataObjectName());
        }
        _ui->verticalLayout_Patch->addWidget(tabWidget);
    }

    void InitialWidget::updateRegions()
    {
        if (_initValue == nullptr)return;

        int count = _initValue->getInitPropRegionCount();

        for (int i = 0; i < count; ++i)
        {
            Interface::FITKOFInitPropRegions * region = _initValue->getInitPropRegionByIndex(i);
            if (!region) continue;
            Core::FITKParameter * paraRegion = region->getRegionPara();
            if (!paraRegion) continue;
            Interface::FITKFluidRegionsMesh* regionMesh = region->getRegionMeshObj();
            if (!regionMesh) continue;
            //创建窗口和垂直分布
            QWidget* w = new QWidget(this);
            _ui->tabWidget_Regions->addTab(w, regionMesh->getDataObjectName());
            QVBoxLayout* pLayout = new QVBoxLayout(w);
            w->setLayout(pLayout);

            for (auto v : paraRegion->getParameter()) {
                if (v == nullptr)continue;
                QWidget* widget = new Core::FITKWidgetComLine(v, this);
                pLayout->addWidget(widget);
            }
            QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            pLayout->addItem(spacer);
        }
    }
}

