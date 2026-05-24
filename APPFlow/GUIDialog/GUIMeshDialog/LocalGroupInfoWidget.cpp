/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "LocalGroupInfoWidget.h"
#include "ui_LocalGroupInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"

namespace GUI
{
    LocalGroupInfoWidget::LocalGroupInfoWidget(Interface::FITKGeometryMeshSize * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _obj(obj), _oper(oper)
    {
        _ui = new Ui::LocalGroupInfoWidget();
        _ui->setupUi(this);

        init();
    }

    LocalGroupInfoWidget::~LocalGroupInfoWidget()
    {
        if (_ui)delete _ui;
    }

    void LocalGroupInfoWidget::init()
    {
        if (_obj) {
            setDataToWidget();
        }
    }

    Interface::FITKGeometryMeshSize * LocalGroupInfoWidget::getCurrentGeoMeshObj()
    {
        return _obj;
    }

    bool LocalGroupInfoWidget::checkValue()
    {
        return true;
    }

    void LocalGroupInfoWidget::setDataToWidget()
    {
        if (_obj == nullptr)return;
        int layer = _obj->getLayerNumber();
        _ui->lineEdit_NoLayers->setText(QString::number(layer));

        double exp = _obj->getExpansionRatio();
        _ui->lineEdit_Expansion->setText(QString::number(exp));

        double thick = _obj->getLayerThickness();
        _ui->lineEdit_FirThickness->setText(QString::number(thick));
    }

    void LocalGroupInfoWidget::getDataFromWidget()
    {

    }

    void LocalGroupInfoWidget::on_lineEdit_NoLayers_editingFinished()
    {
        if (_obj == nullptr)return;
        QString layer = _ui->lineEdit_NoLayers->text();
        _obj->setLayerNumber(layer.toInt());
    }

    void LocalGroupInfoWidget::on_lineEdit_Expansion_editingFinished()
    {
        if (_obj == nullptr)return;
        QString exp = _ui->lineEdit_Expansion->text();
        _obj->setExpansionRatio(exp.toDouble());
    }

    void LocalGroupInfoWidget::on_lineEdit_FirThickness_editingFinished()
    {
        if (_obj == nullptr)return;
        QString thick = _ui->lineEdit_FirThickness->text();
        _obj->setLayerThickness(thick.toDouble());
    }
}
