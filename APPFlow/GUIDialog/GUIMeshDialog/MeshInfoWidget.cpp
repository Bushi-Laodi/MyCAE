/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshInfoWidget.h"
#include "ui_MeshInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredMeshVTK.h"

#define MeshObjIndex Qt::UserRole

namespace GUI {

    MeshInfoWidget::MeshInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(FITKAPP->getGlobalData()->getMainWindow()),
        _oper(oper)
    {
        _ui = new Ui::MeshInfoWidget();
        _ui->setupUi(this);
        init();
    }


    MeshInfoWidget::~MeshInfoWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void MeshInfoWidget::init()
    {
        //初始化表格
        _ui->tableWidget_Mesh->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget_Mesh->horizontalHeader()->setStretchLastSection(true);
        //设置只能单选
        _ui->tableWidget_Mesh->setSelectionMode(QAbstractItemView::SingleSelection);
        //隐藏行表头
        _ui->tableWidget_Mesh->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget_Mesh->horizontalHeader()->setVisible(false);
        //获取网格数据
        auto globalData = FITKAPP->getGlobalData();
        if (globalData == nullptr)return;
        Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        if (meshData == nullptr)return;
        //初始化表格数据
        int count = meshData->getDataCount();
        _ui->tableWidget_Mesh->setRowCount(count);
        for (int i = 0; i < count; i++) {
            Interface::FITKFluidRegionsMesh* mesh = meshData->getDataByIndex(i);
            if (!mesh) continue;
            QString meshName = mesh->getDataObjectName();
            if (meshName.isEmpty()) continue;
            _ui->tableWidget_Mesh->setColumnCount(1);
            QTableWidgetItem* item = new QTableWidgetItem(meshName);
            item->setData(MeshObjIndex, i);
            _ui->tableWidget_Mesh->setItem(i, 0, item);
        }
    }

    void MeshInfoWidget::closeEvent(QCloseEvent * event)
    {
        Core::FITKWidget::closeEvent(event);
    }

}
