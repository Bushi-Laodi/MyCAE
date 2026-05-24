/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "SphereInfoWidget.h"
#include "ui_SphereInfoWidget.h"
#include "CompFaceGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelSphere.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>

namespace GUI {

    SphereInfoWidget::SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(nullptr, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true)
    {
        _ui = new Ui::SphereInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(true);
        init();
    }

    SphereInfoWidget::SphereInfoWidget(Interface::FITKAbsGeoModelSphere * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(obj, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false)
    {
        _ui = new Ui::SphereInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(false);
        init();
    }

    SphereInfoWidget::~SphereInfoWidget()
    {
        if (_faceGroupWidget) {
            delete _faceGroupWidget;
            _faceGroupWidget = nullptr;
        }

        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void SphereInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        QString name = "";
        if (_isCreate) {
            name = geometryData->checkName(tr("Sphere-1"));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);
            setDataToWidget();
        }
    }

    void SphereInfoWidget::setCenterPoint(double * point)
    {
        _ui->lineEdit_CenterPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_CenterPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_CenterPoint3->setText(QString::number(point[2]));
    }

    void SphereInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        _faceGroupWidget->setFaceGroupValue(rowIndex, facesId);
    }

    void SphereInfoWidget::closeEvent(QCloseEvent * event)
    {
        GeometryWidgetBase::closeEvent(event);
        _faceGroupWidget->clearGraphHight();
    }

    void SphereInfoWidget::on_pushButton_CenterPoint_clicked()
    {
        if (_oper) {
            _oper->eventProcess(0);
        }
    }

    void SphereInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void SphereInfoWidget::on_pushButton_CreateOrEdit_clicked()
    {
        if (checkValue() == false)return;

        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;

        if (_isCreate) {
            QString name = _ui->lineEdit_Name->text();
            if (geometryData->getDataByName(name)) {
                QMessageBox::warning(this, "", tr("\"%1\" already exists and cannot be overwritten.").arg(name), QMessageBox::Ok);
                return;
            }

            Interface::FITKInterfaceGeometryFactory* geofactory = Interface::FITKInterfaceGeometryFactory::getInstance();
            if (geofactory == nullptr)return;

            _obj = dynamic_cast<Interface::FITKAbsGeoModelSphere*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTSphere));
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->setDataObjectName(name);
            _obj->update();
            geometryData->appendDataObj(_obj);

            //几何关联的网格区域尺寸
            createMeshSizeGeo();

            //创建Default面组
            createDefaultFaceGroup(_obj);
            //模式切换
            switchCreateModel(false);
        }
        else {
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->update();
        }

        if (_oper && _obj) {
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->execProfession();
        }
    }

    bool SphereInfoWidget::checkValue()
    {
        return true;
    }

    void SphereInfoWidget::setDataToWidget()
    {
        Interface::FITKAbsGeoModelSphere* obj = dynamic_cast<Interface::FITKAbsGeoModelSphere*>(_obj);
        if (obj == nullptr)return;

        double centerPoint[3] = { 0,0,0 };
        obj->getLocation(centerPoint);
        _ui->lineEdit_CenterPoint1->setText(QString::number(centerPoint[0]));
        _ui->lineEdit_CenterPoint2->setText(QString::number(centerPoint[1]));
        _ui->lineEdit_CenterPoint3->setText(QString::number(centerPoint[2]));

        double radius = obj->getRadius();
        _ui->lineEdit_Radius->setText(QString::number(radius));

        _faceGroupWidget->setDataToWidget();
    }

    void SphereInfoWidget::getDataFormWidget()
    {
        Interface::FITKAbsGeoModelSphere* obj = dynamic_cast<Interface::FITKAbsGeoModelSphere*>(_obj);
        if (obj == nullptr)return;

        double centerPoint[3] = { 0,0,0 };
        centerPoint[0] = _ui->lineEdit_CenterPoint1->text().toDouble();
        centerPoint[1] = _ui->lineEdit_CenterPoint2->text().toDouble();
        centerPoint[2] = _ui->lineEdit_CenterPoint3->text().toDouble();
        obj->setLocation(centerPoint);

        double radius = 0.0;
        radius = _ui->lineEdit_Radius->text().toDouble();
        obj->setRadius(radius);
    }

    void SphereInfoWidget::switchCreateModel(bool isCreate)
    {
        if (isCreate == true) {
            _ui->pushButton_CreateOrEdit->setText(tr("Create"));
            _isCreate = true;
        }
        else
        {
            //切换为编辑模式
            _ui->pushButton_CreateOrEdit->setText(tr("Edit"));
            _faceGroupWidget = new CompFaceGroupWidget(this, _obj, _oper);
            _ui->verticalLayout_FaceGroup->addWidget(_faceGroupWidget);
            _ui->lineEdit_Name->setEnabled(false);
            _isCreate = false;
        }
    }
}
