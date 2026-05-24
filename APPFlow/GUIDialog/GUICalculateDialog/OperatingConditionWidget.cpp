/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatingConditionWidget.h"
#include "ui_OperatingConditionWidget.h"

#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFOperatingConditions.h"

namespace GUI
{
    OperatingConditionWidget::OperatingConditionWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::OperatingConditionWidget();
        _ui->setupUi(this);

        init();
    }

    OperatingConditionWidget::~OperatingConditionWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void OperatingConditionWidget::init()
    {
        if (_physicsData == nullptr)return;
        _operCondition = _physicsData->getOperatingConditions();
        updateWidget();
    }

    void OperatingConditionWidget::updateWidget()
    {
        if (_operCondition == nullptr)return;
        _ui->groupBox_GraAcc->hide();
        _ui->groupBox_RefPre->hide();
        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_GraAcc->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
        while ((item = _ui->verticalLayout_RefPre->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

       Core::FITKParameter* graAccData = _operCondition->getGravitationalAcceleration();
       if (graAccData) {
           if (graAccData->getDataCount() > 0)_ui->groupBox_GraAcc->show();
           for (auto d : graAccData->getParameter()) {
               if (d == nullptr)continue;
               QWidget* widget = new Core::FITKWidgetComLine(d, this);
               _ui->verticalLayout_GraAcc->addWidget(widget);
           }
       }

       Core::FITKParameter* refPreData = _operCondition->getReferencePressure();
       if (refPreData) {
           if (refPreData->getDataCount() > 0)_ui->groupBox_RefPre->show();
           for (auto d : refPreData->getParameter()) {
               if (d == nullptr)continue;
               QWidget* widget = nullptr;
               auto type = d->getParamType();
               switch (type){
               case Core::FITKAbstractEasyParam::FITKEasyParamType::FEPRadioGroup:
               case Core::FITKAbstractEasyParam::FITKEasyParamType::FEPBoolGroup: {
                   widget = Core::FITKEasyParamWidgetFactory::createWidget(d, this); break;
                   break;
               }
               default:widget = new Core::FITKWidgetComLine(d, this); break;
               }
               _ui->verticalLayout_RefPre->addWidget(widget);
           }
       }
    }
}

