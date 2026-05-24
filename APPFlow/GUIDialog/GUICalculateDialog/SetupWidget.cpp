/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "SetupWidget.h"
#include "ui_SetupWidget.h"

#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolverManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

#include <QButtonGroup>

#define SetupTypePos "setupWidgetSolverTypePost"
#define SetupType Interface::FITKOFSolverTypeEnum::FITKOFSolverFiltersType

namespace GUI
{
    SetupWidget::SetupWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::SetupWidget();
        _ui->setupUi(this);

        init();
    }

    SetupWidget::~SetupWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void SetupWidget::init()
    {
        _ui->pushButton_SteadyState->setProperty(SetupTypePos, QVariant::fromValue(SetupType::SteadyState));
        _ui->pushButton_SteadyState->setCheckable(true);
        _ui->pushButton_Transient->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Transient));
        _ui->pushButton_Transient->setCheckable(true);
        _ui->pushButton_Compressible->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Compressible));
        _ui->pushButton_Compressible->setCheckable(true);
        _ui->pushButton_Incompressible->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Incompressible));
        _ui->pushButton_Incompressible->setCheckable(true);
        _ui->checkBox_Turbulence->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Turbulences));
        _ui->checkBox_HeatTransfer->setProperty(SetupTypePos, QVariant::fromValue(SetupType::HeatTransfer));
        _ui->checkBox_Radiation->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Radiation));
        _ui->checkBox_Buoyancy->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Buoyancy));
        _ui->checkBox_MRF->setProperty(SetupTypePos, QVariant::fromValue(SetupType::MRF));
        _ui->checkBox_SRF->setProperty(SetupTypePos, QVariant::fromValue(SetupType::SRF));
        _ui->checkBox_Porosity->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Porosity));
        _ui->checkBox_Multiphase->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Multiphase));
        _ui->checkBox_Species->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Spcies));
        _ui->checkBox_Lagrangian->setProperty(SetupTypePos, QVariant::fromValue(SetupType::Lagrangian));
        _ui->checkBox_DynamicMesh->setProperty(SetupTypePos, QVariant::fromValue(SetupType::DynamicMesh));
        _ui->checkBox_UserDefined->setProperty(SetupTypePos, QVariant::fromValue(SetupType::UserDefined));

        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        //自适应
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        initCurrentType();
        initSetupType();
        updateTableWidget();

        QList<QAbstractButton*> radioList = this->findChildren<QAbstractButton*>();
        for (auto radio : radioList) {
            if (radio == nullptr)continue;
            connect(radio, SIGNAL(clicked()), this, SLOT(slotTypeSelect()));
        }
    }

    void SetupWidget::updateTableWidget()
    {
        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);

        for (int i = 0; i < _types.size(); i++) {
            QString name = _types[i];
            QTableWidgetItem* item = new QTableWidgetItem(name);
            _ui->tableWidget->insertRow(i);
            _ui->tableWidget->setItem(i, 0, item);
        }
    }

    void SetupWidget::on_pushButton_Select_clicked()
    {
        if (_solverManager == nullptr)return;
        if (_factoryData == nullptr)return;
        QTableWidgetItem* currentItem = _ui->tableWidget->currentItem();
        if (currentItem == nullptr)return;

        _ui->label_CurrentSolver->setText(currentItem->text());
        _factoryData->setSolver(currentItem->text());

        if (_oper) {
            _oper->execProfession();
        }
    }

    void SetupWidget::initSetupType()
    {
        if (_solverManager == nullptr)return;
        _types = _solverManager->filterSolvers(QList<Interface::FITKOFSolverTypeEnum::FITKOFSolverFiltersType>());
    }

    void SetupWidget::initCurrentType()
    {
        Interface::FITKOFPhysicsData* physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (physicsData == nullptr)return;
        Interface::FITKAbstractOFSolver* solverData = physicsData->getSolver();
        if (solverData == nullptr)return;
        _ui->label_CurrentSolver->setText(solverData->getDataObjectName());
    }

    void SetupWidget::slotTypeSelect()
    {
        QPushButton* senderObj = dynamic_cast<QPushButton*>(sender());
        if (senderObj == _ui->pushButton_SteadyState) {
            bool isChecked = _ui->pushButton_SteadyState->isChecked();
            if (isChecked == true) {
                _ui->pushButton_Transient->setChecked(false);
            }
        }
        else if (senderObj == _ui->pushButton_Transient) {
            bool isChecked = _ui->pushButton_Transient->isChecked();
            if (isChecked == true) {
                _ui->pushButton_SteadyState->setChecked(false);
            }
        }
        else if (senderObj == _ui->pushButton_Incompressible) {
            bool isChecked = _ui->pushButton_Incompressible->isChecked();
            if (isChecked == true) {
                _ui->pushButton_Compressible->setChecked(false);
            }
        }
        else if (senderObj == _ui->pushButton_Compressible) {
            bool isChecked = _ui->pushButton_Compressible->isChecked();
            if (isChecked == true) {
                _ui->pushButton_Incompressible->setChecked(false);
            }
        }

        if (_solverManager == nullptr)return;
        _types.clear();
        QList<SetupType> typeList = {};
        QList<QAbstractButton*> radioList = this->findChildren<QAbstractButton*>();
        for (auto radio : radioList) {
            if (radio == nullptr)continue;
            if (radio->isChecked()) {
                typeList.append(radio->property(SetupTypePos).value<SetupType>());
            }
        }

        _types = _solverManager->filterSolvers(typeList);
        updateTableWidget();
    }
}

