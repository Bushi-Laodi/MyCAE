/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompTranPhasesWidget.h"
#include "ui_CompTranPhasesWidget.h"
#include "CompSelectComBoxWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportProp.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportModel.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportModelManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"


namespace GUI 
{
    Core::FITKParameter* getTranPhasesData(const QString& type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int index = widget->getData("index").toInt();
        phyFactory->setTransportModel(index, type);
        auto tranPhaeseObj = phyData->getTransportProp()->getPhase(index);
        if (tranPhaeseObj == nullptr)return nullptr;
        return tranPhaeseObj->getTransportModel()->getTransportModelPara();
    }

    CompTranPhasesWidget::CompTranPhasesWidget(Interface::FITKOFTransportPhase * phaseData, int index, QWidget * parent) :
        GUICalculateWidgetBase(nullptr, parent), _index(index), _phaseData(phaseData)
    {
        _ui = new Ui::CompTranPhasesWidget();
        _ui->setupUi(this);

        init();
    }

    CompTranPhasesWidget::~CompTranPhasesWidget()
    {
        if (_ui)delete _ui;
    }

    void CompTranPhasesWidget::init()
    {
        if (_phaseData == nullptr)return;
        if (_physicsManager == nullptr)return;
        auto tranManager = _physicsManager->getTransportModelManager();
        if (tranManager == nullptr)return;
        
        Core::FITKParameter* otherData = _phaseData->getPhaseAdditionalData();
        if (otherData) {
            for (auto data : otherData->getParameter()) {
                if (data == nullptr)continue;
                QWidget* widget = new Core::FITKWidgetComLine(data, this);
                _ui->verticalLayout_Sub->addWidget(widget);
            }
        }

        Interface::FITKAbsOFTransportModel* tranModelData = _phaseData->getTransportModel();
        if (tranModelData) {
            CompSelectComBoxWidget* widget = new CompSelectComBoxWidget("Transport Model", this);
            QStringList optians = tranManager->filterTransportModels(_physicsData->getSolver()->getSolverType());
            widget->setData("index", _index);
            widget->setFunction(&getTranPhasesData);
            widget->setOptions(optians);
            widget->setCurrentText(tranModelData->getDataObjectName());
            widget->setSubWidgetData(tranModelData->getTransportModelPara());
            _ui->verticalLayout_Sub->addWidget(widget);
        }
    }
}
