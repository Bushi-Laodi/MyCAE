/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "BoolInfoWidget.h"
#include "ui_BoolInfoWidget.h"
#include "CompFaceGroupSelectWidget.h"
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
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpacerItem>

namespace GUI 
{
    BoolInfoWidget::BoolInfoWidget(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GeometryWidgetBase(obj, oper, dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _isCreate(false)
    {
        init();
        _ui->lineEdit_Name->setEnabled(false);
    }

    BoolInfoWidget::~BoolInfoWidget()
    {
        _faceGroupWidget->clearGraphHight();

        if (_faceGroupWidget == nullptr) {
            delete _faceGroupWidget;
            _faceGroupWidget = nullptr;
        }
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void BoolInfoWidget::init()
    {
        _ui = new Ui::BoolInfoWidget();
        _ui->setupUi(this);

        if (_obj && _oper) {
            QString name = _obj->getDataObjectName();
            _ui->lineEdit_Name->setText(name);

            _faceGroupWidget = new CompFaceGroupWidget(this, _obj, _oper);
            _ui->verticalLayout_FaceGroup->addWidget(_faceGroupWidget);
        }
    }

    void BoolInfoWidget::setFaceGroupValue(int rowIndex, QList<int> facesId)
    {
        _faceGroupWidget->setFaceGroupValue(rowIndex, facesId);
    }

    void BoolInfoWidget::closeEvent(QCloseEvent * event)
    {
        GeometryWidgetBase::closeEvent(event);
        _faceGroupWidget->clearGraphHight();
    }

    bool BoolInfoWidget::checkValue()
    {
        auto outputMessage = [&](QString name, double value) {
            QMessageBox::critical(this, "", tr("%1 value : %2 error!").arg(name).arg(value), QMessageBox::Ok);
        };
        return true;
    }

    void BoolInfoWidget::setDataToWidget()
    {
        _faceGroupWidget->setDataToWidget();
    }
}
