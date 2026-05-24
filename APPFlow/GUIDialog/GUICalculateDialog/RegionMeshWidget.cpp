/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "RegionMeshWidget.h"
#include "ui_RegionMeshWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include <QComboBox>

#define MeshObjID Qt::UserRole

namespace GUI {
    /**
     * @brief    设置区域网格类型数据
     * @param[i] regionMeshID 
     * @param[i] regionMeshtype 
     * @return   void
     * @author   liuzhonghua (liuzhonghuaszch@163.com)
     * @date     2025-06-24
     */
    void setRegionMeshTypeData(int regionMeshID, Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType regionMeshType)
    {
        //获取物理数据搬运工厂
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return;
        //设置区域网格类型
        phyFactory->setRegionMeshType(regionMeshID, regionMeshType);
    }

    RegionMeshWidget::RegionMeshWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(FITKAPP->getGlobalData()->getMainWindow()),
        _oper(oper)
    {
        _ui = new Ui::RegionMeshWidget();
        _ui->setupUi(this);
        init();
    }


    RegionMeshWidget::~RegionMeshWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void RegionMeshWidget::init()
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
        Interface::FITKOFPhysicsData* physicsData = globalData->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (meshData == nullptr || physicsData == nullptr)return;
        //初始化表格数据
        int count = meshData->getDataCount();
        _ui->tableWidget_Mesh->setRowCount(count);
        for (int i = 0; i < count; i++) {
            Interface::FITKFluidRegionsMesh* mesh = meshData->getDataByIndex(i);
            if (!mesh) continue;
            int regionMeshID = mesh->getDataObjectID();
            QString meshName = mesh->getDataObjectName();
            if (meshName.isEmpty()) continue;
            int type = physicsData->getRegionMeshType(mesh->getDataObjectID());
            _ui->tableWidget_Mesh->setColumnCount(2);
            QTableWidgetItem* item = new QTableWidgetItem(meshName);
            item->setData(MeshObjID, regionMeshID);
            _ui->tableWidget_Mesh->setItem(i, 0, item);

            QComboBox* comboBox_Region = new QComboBox;
            comboBox_Region->addItem(tr("None"), (int)Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::None);
            comboBox_Region->addItem(tr("Fluid"), (int)Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::Fluid);
            comboBox_Region->addItem(tr("Solid"), (int)Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::Solid);
            comboBox_Region->setCurrentIndex(comboBox_Region->findData(type));
            if (count == 1)
                comboBox_Region->setEnabled(false);
            connect(comboBox_Region, QOverload<int>::of(&QComboBox::activated), [this, comboBox_Region, regionMeshID](int index) {
                Q_UNUSED(index);
                Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType regionType = (Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType)comboBox_Region->currentData().toInt();
                setRegionMeshTypeData(regionMeshID, regionType);
                //类型为空时会更新setUp树节点
                if (Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::None == regionType && _oper)
                    _oper->execProfession();
            });
            _ui->tableWidget_Mesh->setCellWidget(i, 1, comboBox_Region);
        }
    }

    void RegionMeshWidget::closeEvent(QCloseEvent * event)
    {
        Core::FITKWidget::closeEvent(event);
    }

}
