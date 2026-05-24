/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GeometryDeleteDialog.h"
#include "ui_GeometryDeleteDialog.h"
#include "CudeInfoWidget.h"
#include "CylinderInfoWidget.h"
#include "SphereInfoWidget.h"
#include "BoolInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

namespace GUI
{
    GeometryDeleteDialog::GeometryDeleteDialog(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIDialogBase(FITKAPP->getGlobalData()->getMainWindow()), _oper(oper), _obj(obj)
    {
        _ui = new Ui::GeometryDeleteDialog();
        _ui->setupUi(this);

        if (_obj) {
            QString text = tr("Delete %1").arg(_obj->getDataObjectName());
            _ui->label_Name->setText(text);
        }

        setWindowTitle(tr("Geometry Delete"));
    }

    GeometryDeleteDialog::~GeometryDeleteDialog()
    {
        if (_ui)delete _ui;
    }

    void GeometryDeleteDialog::on_pushButton_OK_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        auto meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        if (meshSizeManager == nullptr)return;

        //判断删除的数据是否是当前界面
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        //所要删除的几何对象ID
        int deleteObjId = _obj->getDataObjectID();

        //获取当前的几何界面与几何id
        GUI::GeometryWidgetBase* currentWidget = dynamic_cast<GUI::GeometryWidgetBase*>(propertyWidget->getCurrentWidget());
        if (currentWidget) {
            //当前显示界面几何id
            int CurrentWidgetGeoID = currentWidget->getCurrentGeoCommand()->getDataObjectID();
            //如果删除的数据是当前界面,删除当前界面
            if (CurrentWidgetGeoID == _obj->getDataObjectID()) {
                propertyWidget->init();
            }
        }

        //清除与当前几何相关的网格边界参数类
        QList<int> meshSizeIds = {};
        for (int i = 0; i < meshSizeManager->getDataCount(); i++) {
            auto meshSize = meshSizeManager->getDataByIndex(i);
            if(meshSize == nullptr)continue;
            if (meshSize->getGeoModel() == _obj) {
                meshSizeIds.append(meshSize->getDataObjectID());
            }
        }
        for (int id : meshSizeIds) {
            meshSizeManager->removeDataByID(id);
        }
        
        //清除当前几何关联的网格区域尺寸
        auto RegionMeshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        for (auto RegionMeshSize : RegionMeshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom))
        {
            auto RegionGeoMeshSize = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>(RegionMeshSize);
            if (RegionGeoMeshSize == nullptr)continue;
            if (RegionGeoMeshSize->getGeomID() == _obj->getDataObjectID()) {
                RegionMeshSizeManager->removeDataObj(RegionGeoMeshSize);
                break;
            }
        }

        //刷新几何关联的网格区域尺寸界面
        EventOper::ParaWidgetInterfaceOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::ParaWidgetInterfaceOperator>("actionMeshGeoDelete");
        if (graphOper) {
            QObject* object = new QObject();
            object->setObjectName("actionMeshGeoDelete");
            graphOper->setEmitter(object);
            graphOper->setArgs("objID", deleteObjId);
            graphOper->actionTriggered();
        }

        //刷求解器中对应几何的patch节点
        graphOper = FITKOPERREPO->getOperatorT<EventOper::ParaWidgetInterfaceOperator>("actionInitialDeleteGeo");
        if (graphOper) {
            QObject* object = new QObject();
            object->setObjectName("actionInitialDeleteGeo");
            graphOper->setEmitter(object);
            graphOper->setArgs("objID", deleteObjId);
            graphOper->actionTriggered();
        }

        //清除几何对象
        geometryData->removeDataByID(deleteObjId);
        _oper->execProfession();
        this->accept();
    }

    void GeometryDeleteDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }
}
