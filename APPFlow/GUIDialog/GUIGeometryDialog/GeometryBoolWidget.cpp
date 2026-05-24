/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GeometryBoolWidget.h"
#include "ui_GeometryBoolWidget.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoOperBool.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"

#include <QMessageBox>

namespace GUI
{
    GeometryBoolWidget::GeometryBoolWidget(BoolType type, EventOper::ParaWidgetInterfaceOperator* oper) :
        GeometryWidgetBase(nullptr, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _type(type), _oper(oper)
    {
        _ui = new Ui::GeometryBoolWidget();
        _ui->setupUi(this);

        init();

        _ui->pushButton_Body1->hide();
        _ui->pushButton_Body2->hide();

        QString typeName = "";
        switch (_type) {
        case GUI::BoolType::GeoBoolFause: typeName = tr("Fause"); break;
        case GUI::BoolType::GeoBoolCut:typeName = tr("Cut"); break;
        case GUI::BoolType::GeoBoolCommon:typeName = tr("Common"); break;
        }
        _ui->groupBox_Type->setTitle(typeName);


        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        QString name = geometryData->checkName(tr("Bool-1"));
        _ui->lineEdit_Name->setText(name);
    }

    GeometryBoolWidget::~GeometryBoolWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void GeometryBoolWidget::init()
    {
        _ui->comboBox_Body1->clear();
        _ui->comboBox_Body2->clear();
        _ui->comboBox_Body1->addItem(tr("None"), -1);
        _ui->comboBox_Body2->addItem(tr("None"), -1);

        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        for (int i = 0; i < geometryData->getDataCount(); i++) {
            Interface::FITKAbsGeoCommand* geoObj = geometryData->getDataByIndex(i);
            if (geoObj == nullptr)continue;
            QString name = geoObj->getDataObjectName();
            int id = geoObj->getDataObjectID();
            _ui->comboBox_Body1->addItem(name, id);
            _ui->comboBox_Body2->addItem(name, id);
        }
    }

    void GeometryBoolWidget::on_comboBox_Body1_activated(int index)
    {
        int box1Id = _ui->comboBox_Body1->currentData().toInt();
        int box2Id = _ui->comboBox_Body2->currentData().toInt();

        if (box2Id == box1Id) {
            _ui->comboBox_Body2->setCurrentIndex(0);
        }
    }

    void GeometryBoolWidget::on_comboBox_Body2_activated(int index)
    {
        int box1Id = _ui->comboBox_Body1->currentData().toInt();
        int box2Id = _ui->comboBox_Body2->currentData().toInt();

        if (box2Id == box1Id) {
            _ui->comboBox_Body1->setCurrentIndex(0);
        }
    }

    void GeometryBoolWidget::on_pushButton_Body1_clicked()
    {

    }

    void GeometryBoolWidget::on_pushButton_Body2_clicked()
    {

    }

    void GeometryBoolWidget::on_pushButton_Apply_clicked()
    {
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        Interface::FITKInterfaceGeometryFactory* geoFactory = Interface::FITKInterfaceGeometryFactory::getInstance();
        if (geoFactory == nullptr)return;

        if (_oper == nullptr)return;
        if (!checkValue())return;

        Interface::FITKAbsGeoCommand* geo1 = geometryData->getDataByID(_ui->comboBox_Body1->currentData().toInt());
        Interface::FITKAbsGeoCommand* geo2 = geometryData->getDataByID(_ui->comboBox_Body2->currentData().toInt());
        if (geo1 == nullptr)return;
        if (geo2 == nullptr)return;

        auto geoBoolOper = geoFactory->createCommandT<Interface::FITKAbsGeoOperBool>(Interface::FITKGeoEnum::FITKGeometryComType::FGTBool);
        if (geoBoolOper == nullptr)return;

        geoBoolOper->setDataObjectName(_ui->lineEdit_Name->text());
        switch (_type) {
        case GUI::BoolType::GeoBoolFause:
            geoBoolOper->setBoolOperType(Interface::FITKAbsGeoOperBool::GBTAdd);
            break;
        case GUI::BoolType::GeoBoolCut:
            geoBoolOper->setBoolOperType(Interface::FITKAbsGeoOperBool::GBTCut);
            break;
        case GUI::BoolType::GeoBoolCommon:
            geoBoolOper->setBoolOperType(Interface::FITKAbsGeoOperBool::GBTCommon);
            break;
        }
        geoBoolOper->setGeoShape1(geo1->getShapeAgent());
        geoBoolOper->setGeoShape2(geo2->getShapeAgent());
        geoBoolOper->update();

        _obj = geoBoolOper;

        //创建Default面组
        createDefaultFaceGroup(_obj);
        //几何关联的网格区域尺寸
        createMeshSizeGeo();
        geometryData->appendDataObj(geoBoolOper);

        //隐藏原模型
        geo1->enable(false);
        geo2->enable(false);

        if (_oper) {
            _oper->setArgs("geo1ObjID", geo1->getDataObjectID());
            _oper->setArgs("geo2ObjID", geo2->getDataObjectID());
            _oper->setArgs("objID", geoBoolOper->getDataObjectID());
            _oper->execProfession();

            QString actionName = "actionGeoBoolEdit";
            QObject sender;
            sender.setObjectName(actionName);
            auto acOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<Core::FITKActionOperator>(actionName);
            if (acOper == nullptr)return;
            acOper->setEmitter(&sender);
            acOper->setArgs("objID", geoBoolOper->getDataObjectID());
            acOper->actionTriggered();
        }
    }

    bool GeometryBoolWidget::checkValue()
    {
        int box1Id = _ui->comboBox_Body1->currentData().toInt();
        int box2Id = _ui->comboBox_Body2->currentData().toInt();

        auto outputMessage = [&](QString message) {
            QMessageBox::warning(this, tr("Warring"), message, QMessageBox::Ok);
        };

        if (box1Id == -1 || box2Id == -1) {
            outputMessage(tr("Input Wrong!"));
            return false;
        }

        if (box1Id == box2Id) {
            outputMessage(tr("Input Wrong!"));
            return false;
        }

        QString name = _ui->lineEdit_Name->text();
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData->getDataByName(name)) {
            outputMessage(tr("%1 exist!").arg(name));
            return false;
        }
        return true;
    }
}