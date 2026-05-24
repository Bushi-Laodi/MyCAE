/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CylinderInfoWidget.h"
#include "ui_CylinderInfoWidget.h"
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
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelCylinder.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QtMath>

namespace GUI {

    CylinderInfoWidget::CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(nullptr, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(true)
    {
        _ui = new Ui::CylinderInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(true);
        init();
    }

    CylinderInfoWidget::CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(obj, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false)
    {
        _ui = new Ui::CylinderInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(false);
        init();
    }

    CylinderInfoWidget::~CylinderInfoWidget()
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

    void CylinderInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        QString name = "";
        if (_isCreate) {
            name = geometryData->checkName(tr("Cylinder-1"));
            _ui->lineEdit_Name->setText(name);
        }
        else
        {
            name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);
            setDataToWidget();
        }

        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icoR_selectBlue.png"), QSize(), QIcon::Normal, QIcon::Off);
        _ui->pushButton_OriginPoint->setIcon(icon1);
    }

    void CylinderInfoWidget::setOriginPoint(double * point)
    {
        _ui->lineEdit_OriginPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(point[2]));
    }

    void CylinderInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        _faceGroupWidget->setFaceGroupValue(rowIndex, facesId);
    }

    void CylinderInfoWidget::closeEvent(QCloseEvent * event)
    {
        GeometryWidgetBase::closeEvent(event);
        _faceGroupWidget->clearGraphHight();
    }

    void CylinderInfoWidget::on_pushButton_OriginPoint_clicked()
    {
        if (_oper) {
            _oper->eventProcess(0);
        }
    }

    void CylinderInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void CylinderInfoWidget::on_pushButton_CreateOrEdit_clicked()
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

            _obj = dynamic_cast<Interface::FITKAbsGeoModelCylinder*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTCylinder));
            if (_obj == nullptr)return;
            getDataFormWidget();
            _obj->setDataObjectName(name);
            _obj->update();
            geometryData->appendDataObj(_obj);

            //几何关联的网格区域尺寸
            createMeshSizeGeo();

            //创建Default面组
            createDefaultFaceGroup(_obj);
            //切换为编辑模式
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

    bool CylinderInfoWidget::checkValue()
    {
        auto outputMessage = [&](QString message) {
            QMessageBox::critical(nullptr, tr("Error Information"), message);
        };

        double axisPoint1 = _ui->lineEdit_Axis1->text().toDouble();
        double axisPoint2 = _ui->lineEdit_Axis2->text().toDouble();
        double axisPoint3 = _ui->lineEdit_Axis3->text().toDouble();
        if (axisPoint1 == 0 && axisPoint2 == 0 && axisPoint3 == 0) {
            outputMessage(tr("AxisX = 0 , AxisY = 0 , AxisZ = 0"));
            return false;
        }
        else
        {
            double mulRatio = qSqrt(axisPoint1*axisPoint1 + axisPoint2 * axisPoint2 + axisPoint3 * axisPoint3);
            _ui->lineEdit_Axis1->setText(QString::number(axisPoint1 / mulRatio));
            _ui->lineEdit_Axis2->setText(QString::number(axisPoint2 / mulRatio));
            _ui->lineEdit_Axis3->setText(QString::number(axisPoint3 / mulRatio));
        }

        return true;
    }

    void CylinderInfoWidget::setDataToWidget()
    {
        Interface::FITKAbsGeoModelCylinder* obj = dynamic_cast<Interface::FITKAbsGeoModelCylinder*>(_obj);
        if (obj == nullptr)return;

        double originPoint[3] = { 0,0,0 };
        obj->getLocation(originPoint);
        _ui->lineEdit_OriginPoint1->setText(QString::number(originPoint[0]));
        _ui->lineEdit_OriginPoint2->setText(QString::number(originPoint[1]));
        _ui->lineEdit_OriginPoint3->setText(QString::number(originPoint[2]));

        double axis[3] = { 0,0,0 };
        obj->getDirection(axis);
        _ui->lineEdit_Axis1->setText(QString::number(axis[0]));
        _ui->lineEdit_Axis2->setText(QString::number(axis[1]));
        _ui->lineEdit_Axis3->setText(QString::number(axis[2]));

        double radius = obj->getRadius();
        _ui->lineEdit_Radius->setText(QString::number(radius));

        double length = obj->getLength();
        _ui->lineEdit_Length->setText(QString::number(length));

        _faceGroupWidget->setDataToWidget();
    }

    void CylinderInfoWidget::getDataFormWidget()
    {
        Interface::FITKAbsGeoModelCylinder* obj = dynamic_cast<Interface::FITKAbsGeoModelCylinder*>(_obj);
        if (obj == nullptr)return;

        double originPoint[3] = { 0,0,0 };
        originPoint[0] = _ui->lineEdit_OriginPoint1->text().toDouble();
        originPoint[1] = _ui->lineEdit_OriginPoint2->text().toDouble();
        originPoint[2] = _ui->lineEdit_OriginPoint3->text().toDouble();
        obj->setLocation(originPoint);

        double axis[3] = { 0,0,0 };
        axis[0] = _ui->lineEdit_Axis1->text().toDouble();
        axis[1] = _ui->lineEdit_Axis2->text().toDouble();
        axis[2] = _ui->lineEdit_Axis3->text().toDouble();
        obj->setDirection(axis);

        double radius = _ui->lineEdit_Radius->text().toDouble();
        obj->setRadius(radius);

        double length = _ui->lineEdit_Length->text().toDouble();
        obj->setLength(length);
    }

    void CylinderInfoWidget::switchCreateModel(bool isCreate)
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
