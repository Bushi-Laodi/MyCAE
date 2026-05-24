/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MaterialPointWidget.h"
#include "ui_MaterialPointWidget.h"
#include "CompMaterialPointWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIFrame/MainTreeWidget.h"
#include "GUIWidget/PickedData.h"
#include "GUIWidget/PickedDataProvider.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKZonePoints.h"

#include <QDebug>

#define MatPointID Qt::UserRole + 1
#define MatPointName Qt::UserRole + 1875

namespace GUI
{
    MaterialPointWidget::MaterialPointWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::MaterialPointWidget();
        _ui->setupUi(this);
        init();
    }

    MaterialPointWidget::~MaterialPointWidget()
    {
        clearTableWidget();
        if (_ui)delete _ui;
    }

    void MaterialPointWidget::init()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;
        auto pointObj = manager->getDataByIndex(0);
        if (pointObj) {
            pointObj->setUserData(MatPointName, tr("none"));
        }

        initTableWidget();
        //数据填充
        setDataToWidget();
    }

    void MaterialPointWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        QStringList header;
        header << tr("");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        //隐藏列表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏行表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void MaterialPointWidget::hideEvent(QHideEvent * event)
    {
        updateGraph(false);
        clearGraphHight();
    }

    void MaterialPointWidget::on_pushButton_Clear_clicked()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;

        for (int i = 1; i < _ui->tableWidget->rowCount(); i++) {
            auto widget = dynamic_cast<CompMaterialPointWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if(widget == nullptr)continue;
            manager->removeDataByID(widget->data(MatPointID).toInt());
        }
        updateTableWidget();
        updateGraph();
    }

    void MaterialPointWidget::on_pushButton_Add_clicked()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;

        double point[3] = { 0,0,0 };
        Interface::FITKZonePoint* zonePoint = new Interface::FITKZonePoint(point[0], point[1], point[2]);
        manager->appendDataObj(zonePoint);
        int i = 1;
        QString name = getPointNewName();
        zonePoint->setUserData(MatPointName, name);

        updateTableWidget();
        updateGraph();
    }

    void MaterialPointWidget::slotCellTableClicked(int row, int column)
    {
        CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(_ui->tableWidget->cellWidget(row, 0));
        if (widget == nullptr) return;

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        GraphOperParam param;
        param.AdvHighlightIndice.append(widget->data(MatPointID).toInt());
        param.HighlightMode = HighlightLevel::AdvHighlight;
        graphOper->updateGraphByType(static_cast<int>(GUI::MainTreeEnum::MainTree_MeshPoint), param);
        graphOper->reRender();
    }

    void MaterialPointWidget::slotMatPointWidgetDeleteClicked()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;

        //如果只剩一个点，无法删除
        if (manager->getDataCount() == 1)return;

        CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(sender());
        if (widget == nullptr) return;
        int objId = widget->data(MatPointID).toInt();
        auto pointObj = manager->getDataByID(objId);
        if (pointObj == nullptr)return;
        manager->removeDataByID(objId);

        updateTableWidget();
        updateGraph();
    }

    void MaterialPointWidget::slotMatPointWidgetPointChange()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;
        CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(sender());
        if (widget == nullptr) return;
        auto pointObj = manager->getDataByID(widget->data(MatPointID).toInt());
        if (pointObj == nullptr)return;
        
        double point[3] = { 0,0,0 };
        widget->getPoint(point);
        pointObj->setCoor(point[0], point[1], point[2]);

        updateGraph();
    }

    QString MaterialPointWidget::getPointNewName()
    {
        int maxIndex = 0;
        QString name = QString(tr("zone1"));
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return name;
        
        for (int i = 1; i < manager->getDataCount(); i++)
        {
            auto obj = manager->getDataByIndex(i);
            if (obj == nullptr)continue;
            QString objName = obj->getUserData(MatPointName).toString();
            int index = objName.mid(4, objName.size()).toInt();
            if (maxIndex < index) maxIndex = index;
        }

        maxIndex++;
        name = QString(tr("zone%1")).arg(maxIndex);
        return name;
    }

    void MaterialPointWidget::updateTableWidget()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;

        //清楚旧数据
        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);

        //添加新数据
        _ui->tableWidget->setRowCount(manager->getDataCount());
        for (int i = 0; i < manager->getDataCount(); i++) {
            auto pointObj = manager->getDataByIndex(i);
            if (pointObj == nullptr)continue;

            QString name = pointObj->getUserData(MatPointName).toString();
            double point[3] = { 0,0,0 };
            pointObj->getCoor(point);
            CompMaterialPointWidget* widget = new CompMaterialPointWidget(_ui->tableWidget);
            widget->setName(name);
            widget->setPoint(point);
            _ui->tableWidget->setCellWidget(i, 0, widget);
            widget->setData(MatPointID, pointObj->getDataObjectID());

            connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotMatPointWidgetDeleteClicked()));
            connect(widget, SIGNAL(sigPointChange()), this, SLOT(slotMatPointWidgetPointChange()));
        }

        updateFaceWidgetCurrentPos();
    }

    void MaterialPointWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setCurrentPos(i, 0);
        }
    }

    void MaterialPointWidget::clearGraphHight()
    {
        //退出选择模式
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;
        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        GUI::GUIPickInfo::SetPickInfo(pinfo);
        pickD->clearPickedData();

        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void MaterialPointWidget::clearTableWidget()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            QWidget* widget = _ui->tableWidget->cellWidget(i, 0);
            if (widget == nullptr)continue;
            delete widget;
            widget = nullptr;
        }

        _ui->tableWidget->clear();
    }
    bool MaterialPointWidget::checkValue()
    {
        return true;
    }

    void MaterialPointWidget::setDataToWidget()
    {
        Interface::FITKMeshGenInterface* interface = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKZonePointManager* manager = interface->getZonePointManager();
        if (manager == nullptr)return;

        updateTableWidget();
        updateGraph();
    }

    void MaterialPointWidget::getDataFromWidget()
    {

    }

    void MaterialPointWidget::updateGraph(bool isShow)
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        GraphOperParam param;
        param.Visibility = isShow;
        graphOper->updateGraphByType(static_cast<int>(GUI::MainTreeEnum::MainTree_MeshPoint), param);
        graphOper->reRender();
    }
}

