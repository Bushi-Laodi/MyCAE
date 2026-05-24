/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "LocalSelectGroupWidget.h"
#include "ui_LocalSelectGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoModelManager.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractGeometryMeshSizeGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"

#define LocalGeoId Qt::UserRole
#define LocalGroId Qt::UserRole + 1

namespace GUI
{
    LocalSelectGroupWidget::LocalSelectGroupWidget(EventOper::ParaWidgetInterfaceOperator* oper) :
        GUIWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::LocalSelectGroupWidget();
        _ui->setupUi(this);

        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        //自适应
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        init();
    }

    LocalSelectGroupWidget::~LocalSelectGroupWidget()
    {
        if (_ui)delete _ui;
    }

    void LocalSelectGroupWidget::init()
    {
        updateTableWidget();
    }

    void LocalSelectGroupWidget::updateTableWidget()
    {
        Interface::FITKMeshGenInterface* genInterface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKGeometryMeshSizeManager* manger = genInterface->getGeometryMeshSizeManager();
        if (manger == nullptr)return;
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;

        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);
        int currentRow = 0;

        //数据是否存在在管理器中局部函数
        auto isInclude = [&](int id)->bool {
            for (int i = 0; i < manger->getDataCount(); i++) {
                auto obj = manger->getDataByIndex(i);
                if (obj == nullptr)continue;
                if (obj->getGeoGroupComponentId() == id)return true;
            }
            return false;
        };

        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geometryObj = geometryData->getDataByIndex(i);
            if (geometryObj == nullptr)continue;
            if (geometryObj->getShapeAgent() == nullptr)continue;

            Interface::FITKGeoComponentManager* compManager = geometryObj->getShapeAgent()->getGeoComponentManager();
            if (compManager == nullptr)continue;
            QString geoName = geometryObj->getDataObjectName();
            for (int j = 0; j < compManager->getDataCount(); j++) {
                Interface::FITKGeoComponent* geoCom = compManager->getDataByIndex(j);
                if (geoCom == nullptr)continue;
                //判断数据是否在管理器中存在
                if (isInclude(geoCom->getDataObjectID()))continue;
                //不存在添加到选项列表中
                QString comName = geoCom->getDataObjectName();
                QTableWidgetItem* item = new QTableWidgetItem(geoName + "." + comName);
                item->setData(LocalGeoId, geometryObj->getDataObjectID());
                item->setData(LocalGroId, geoCom->getDataObjectID());
                _ui->tableWidget->insertRow(currentRow);
                _ui->tableWidget->setItem(currentRow, 0, item);
                currentRow++;
            }
        }
    }

    void LocalSelectGroupWidget::on_pushButton_Add_clicked()
    {
        QList<QTableWidgetItem*> items = _ui->tableWidget->selectedItems();
        for (QTableWidgetItem* item : items) {
            if (item == nullptr)return;

            Interface::FITKMeshGenInterface* genInterface = Interface::FITKMeshGenInterface::getInstance();
            Interface::FITKAbstractGeometryMeshSizeGenerator* generator = genInterface->getGeometryMeshSizeGenerator();
            if (generator == nullptr)return;
            Interface::FITKGeometryMeshSizeManager* manger = genInterface->getGeometryMeshSizeManager();
            if (manger == nullptr)return;

            //创建几何划分网格尺寸类
            Interface::FITKGeometryMeshSize* meshSize = generator->createGeometryMeshSize();
            meshSize->setGeoGroupComponentId(item->data(LocalGroId).toInt());
            meshSize->setDataObjectName(item->text());

            //添加至管理器中
            manger->appendDataObj(meshSize);
        }

        if (_oper) {
            _oper->execProfession();
            updateTableWidget();
        }
    }
}

