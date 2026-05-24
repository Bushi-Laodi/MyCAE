/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshGeoWidget.h"
#include "ui_MeshGeoWidget.h"
#include "MeshGeoSubWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"

#include <QVBoxLayout>
#include <QSpacerItem>

namespace GUI
{
    MeshGeoWidget::MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::MeshGeoWidget();
        _ui->setupUi(this);
        init();
    }

    MeshGeoWidget::~MeshGeoWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void MeshGeoWidget::init()
    {
        updateWidget();
    }

    void MeshGeoWidget::updateWidget()
    {
        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geoData = geometryData->getDataByIndex(i);
            if (geoData == nullptr)continue;
            MeshGeoSubWidget* subWidget = new MeshGeoSubWidget(geoData, _oper, tabWidget);
            tabWidget->addTab(subWidget, geoData->getDataObjectName());
        }
        _ui->verticalLayout->addWidget(tabWidget);
    }
}

