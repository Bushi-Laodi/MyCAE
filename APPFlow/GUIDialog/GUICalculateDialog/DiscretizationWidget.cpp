/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "DiscretizationWidget.h"
#include "ui_DiscretizationWidget.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFDiscretization.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"

#include <QToolBox>
#include <QTabBar>

namespace GUI
{
    DiscretizationWidget::DiscretizationWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::DiscretizationWidget();
        _ui->setupUi(this);

        init();
    }

    DiscretizationWidget::~DiscretizationWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void DiscretizationWidget::init()
    {
        if (_physicsData == nullptr)return;
        _disValue = _physicsData->getDiscretization();
        updateWidget();
    }

    void DiscretizationWidget::updateWidget()
    {
        updateTimeWidget();
        updateConvectionWidget();
        updateGradientsWidget();
        updateInterpolationWidget();
    }

    void DiscretizationWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void DiscretizationWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void DiscretizationWidget::updateTimeWidget()
    {
        if (_disValue == nullptr)return;
        auto timeValue = _disValue->getTimeOption();
        if (timeValue == nullptr)return;
        
        for (auto dataBase : timeValue->getParameter())
        {
            if (dataBase == nullptr)continue;
            QWidget* widget = Core::FITKEasyParamWidgetFactory::createWidget(dataBase, this);
            _ui->verticalLayout_Time->addWidget(widget);
        }
    }

    void DiscretizationWidget::updateConvectionWidget()
    {
        if (_disValue == nullptr)return;

        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        int conNum = _disValue->getConvectionCount();
        for (int i = 0; i < conNum; i++) {
            auto conValue = _disValue->getConvectionVPara(i);
            QString conName = _disValue->getConvectionVName(i);
            if(conValue == nullptr)continue;

            for (auto subValue : conValue->getParameter()){
                if (subValue == nullptr)continue;
                QWidget* widget = Core::FITKEasyParamWidgetFactory::createWidget(subValue, this);
                if (widget == nullptr) {
                    continue;
                }
                //添加弹簧
                QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
                widget->layout()->addItem(spacer);
                tabWidget->addTab(widget, conName);
            }
        }
        //添加界面
        _ui->verticalLayout_Convection->addWidget(tabWidget);
    }

    void DiscretizationWidget::updateGradientsWidget()
    {
        if (_disValue == nullptr)return;
        auto gradValue = _disValue->getGradients();
        if (gradValue == nullptr)return;

        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        for (auto value : gradValue->getParameter()) {
            if (value == nullptr)continue;
            QWidget* widget = Core::FITKEasyParamWidgetFactory::createWidget(value, this);
            if (widget == nullptr) {
                continue;
            }
            //添加弹簧
            QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            widget->layout()->addItem(spacer);
            tabWidget->addTab(widget, value->getDataObjectName());
        }
        //添加界面
        _ui->verticalLayout_Gradients->addWidget(tabWidget);
    }

    void DiscretizationWidget::updateInterpolationWidget()
    {
        if (_disValue == nullptr)return;
        auto intValue = _disValue->getInterpolation();
        if (intValue == nullptr)return;

        for (auto value : intValue->getParameter()) {
            QWidget* widget = new Core::FITKWidgetComLine(value, this);
            _ui->verticalLayout_Interpolation->addWidget(widget);
        }
    }
}

