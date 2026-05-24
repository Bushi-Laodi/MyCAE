/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CudeInfoWidget.h"
#include "ui_CudeInfoWidget.h"
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
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpacerItem>

namespace GUI {

    CudeInfoWidget::CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(nullptr, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow()))
    {
        _ui = new Ui::CudeInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(true);
        init();
    }

    CudeInfoWidget::CudeInfoWidget(Interface::FITKAbsGeoModelBox * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(obj, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow()))
    {
        _ui = new Ui::CudeInfoWidget();
        _ui->setupUi(this);
        switchCreateModel(false);
        init();
    }

        CudeInfoWidget::~CudeInfoWidget()
    {
        _faceGroupWidget->clearGraphHight();
        if (_faceGroupWidget) {
            delete _faceGroupWidget;
            _faceGroupWidget = nullptr;
        }

        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void CudeInfoWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        QString name = "";
        if (_isCreate) {
            name = geometryData->checkName(tr("Box-1"));
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
        _ui->pushButton_BasicPoint->setIcon(icon1);
    }

    void CudeInfoWidget::setBasicPoint(double * point)
    {
        _ui->lineEdit_BasicPoint1->setText(QString::number(point[0]));
        _ui->lineEdit_BasicPoint2->setText(QString::number(point[1]));
        _ui->lineEdit_BasicPoint3->setText(QString::number(point[2]));
    }

    void CudeInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        _faceGroupWidget->setFaceGroupValue(rowIndex, facesId);
    }

    void CudeInfoWidget::closeEvent(QCloseEvent * event)
    {
        GeometryWidgetBase::closeEvent(event);
        _faceGroupWidget->clearGraphHight();
    }

    void CudeInfoWidget::on_pushButton_BasicPoint_clicked()
    {
        if (_oper) {
            _oper->eventProcess(0);
        }
    }

    void CudeInfoWidget::on_pushButton_Cancel_clicked()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        propertyWidget->init();
    }

    void CudeInfoWidget::on_pushButton_CreateOrEdit_clicked()
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

            _obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(geofactory->createCommand(Interface::FITKGeoEnum::FITKGeometryComType::FGTBox));
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

    bool CudeInfoWidget::checkValue()
    {
        auto outputMessage = [&](QString name, double value) {
            QMessageBox::critical(this, "", tr("%1 value : %2 error!").arg(name).arg(value), QMessageBox::Ok);
        };

        double value = _ui->lineEdit_Dimensions1->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions X"), value);
            return false;
        }

        value = _ui->lineEdit_Dimensions2->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions Y"), value);
            return false;
        }

        value = _ui->lineEdit_Dimensions3->text().toDouble();
        if (value == 0) {
            outputMessage(tr("Dimensions Z"), value);
            return false;
        }

        return true;
    }

    void CudeInfoWidget::setDataToWidget()
    {
        Interface::FITKAbsGeoModelBox* obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(_obj);
        if (obj == nullptr)return;

        double basicPoint[3] = { 0,0,0 };
        obj->getPoint1(basicPoint);
        _ui->lineEdit_BasicPoint1->setText(QString::number(basicPoint[0]));
        _ui->lineEdit_BasicPoint2->setText(QString::number(basicPoint[1]));
        _ui->lineEdit_BasicPoint3->setText(QString::number(basicPoint[2]));

        double dimensions[3] = { 0,0,0 };
        obj->getLength(dimensions);
        _ui->lineEdit_Dimensions1->setText(QString::number(dimensions[0]));
        _ui->lineEdit_Dimensions2->setText(QString::number(dimensions[1]));
        _ui->lineEdit_Dimensions3->setText(QString::number(dimensions[2]));

        _faceGroupWidget->setDataToWidget();
    }

    void CudeInfoWidget::getDataFormWidget()
    {
        Interface::FITKAbsGeoModelBox* obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(_obj);
        if (obj == nullptr)return;

        double basicPoint[3] = { 0,0,0 };
        basicPoint[0] = _ui->lineEdit_BasicPoint1->text().toDouble();
        basicPoint[1] = _ui->lineEdit_BasicPoint2->text().toDouble();
        basicPoint[2] = _ui->lineEdit_BasicPoint3->text().toDouble();
        obj->setPoint1(basicPoint);

        double dimensions[3] = { 0,0,0 };
        dimensions[0] = _ui->lineEdit_Dimensions1->text().toDouble();
        dimensions[1] = _ui->lineEdit_Dimensions2->text().toDouble();
        dimensions[2] = _ui->lineEdit_Dimensions3->text().toDouble();
        obj->setLength(dimensions);
    }

    void CudeInfoWidget::switchCreateModel(bool isCreate)
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
