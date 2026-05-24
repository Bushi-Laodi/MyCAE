/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "TurbulenceWidget.h"
#include "ui_TurbulenceWidget.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTurbulenceManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFTurbulence.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

#include <QButtonGroup>
#include <QScrollArea>

namespace GUI
{
    TurbulenceWidget::TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::TurbulenceWidget();
        _ui->setupUi(this);
        _turManager = _physicsManager->getTurbulenceManager();
        init();
    }

    TurbulenceWidget::~TurbulenceWidget()
    {
        if (_radioGroup) delete _radioGroup;
        if (_ui)  delete _ui;
    }

    void TurbulenceWidget::init()
    {
        //界面默认设置
        _radioGroup = new QButtonGroup();
        _radioGroup->addButton(_ui->radioButton_Laminar);
        _radioGroup->addButton(_ui->radioButton_RANS);
        _radioGroup->addButton(_ui->radioButton_LES);
        _ui->radioButton_Laminar->setChecked(true);
        //默认隐藏湍流模型参数
        _ui->pushButton_ModelUnfold->setCheckable(true);
        _ui->pushButton_DeltaUnfold->setCheckable(true);
        _ui->widget_sub->hide();
        _ui->widget_Model->hide();
        _ui->widget_ModelSub->hide();
        _ui->widget_Delta->hide();
        _ui->widget_DeltaSub->hide();

        //更具数据对象初始化界面
        setDataToWidget();

        //信号连接
        connect(_radioGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioButtonClicked()));
        connect(_ui->comboBox_Model, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboBoxModelChange(int)));
        connect(_ui->comboBox_Delta, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboBoxDeltaChange(int)));
    }

    void TurbulenceWidget::updateSubWidget()
    {
        if (_physicsData == nullptr)return;
        Interface::FITKAbstractOFTurbulence* turData = _physicsData->getTurbulence();
        if (turData == nullptr)return;

        //清除子参数
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_ModelSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }
        while ((item = _ui->verticalLayout_DeltaSub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }

        //重新添加子参数
        if (turData->getTurbulenceParams()) {
            for (int i = 0; i < turData->getTurbulenceParams()->getDataCount(); i++) {
                auto dataBase = turData->getTurbulenceParams()->getDataByIndex(i);
                if (dataBase == nullptr)continue;
                _ui->verticalLayout_ModelSub->addWidget(new Core::FITKWidgetComLine(dataBase, this));
            }
        }
        //if (turData->getDeltaData(turData->getCurrentDeltaType())) {
        //    auto deltaData = _turData->getDeltaData(_ui->comboBox_Delta->currentText());
        //    for (int i = 0; i < deltaData->getParameterCount(); i++) {
        //        auto dataBase = deltaData->getParameterAt(i);
        //        if (dataBase == nullptr)continue;
        //        _ui->verticalLayout_DeltaSub->addWidget(new Core::FITKWidgetComLine(dataBase, this));
        //    }
        //}
    }

    void TurbulenceWidget::setDataToWidget()
    {
        if (_physicsData == nullptr)return;
        auto solverData = _physicsData->getSolver();
        if (solverData == nullptr)return;
        
        //湍流数据是否存在
        Interface::FITKAbstractOFTurbulence* turObj = _physicsData->getTurbulence();
        if (turObj == nullptr)return;
        _ui->widget_sub->show();
        _ui->widget_Model->show();
        
        auto turType = turObj->getTurbulenceType();

        QStringList modelTypes = _turManager->filterTurbence(turType, solverData->getSolverType());
        _ui->comboBox_Model->clear();
        _ui->comboBox_Model->addItems(modelTypes);
        _ui->comboBox_Model->setCurrentText(turObj->getDataObjectName());

        switch (turType){
        case Interface::FITKOFSolverTypeEnum::TurbNone: {
            _ui->radioButton_Laminar->setChecked(true);
            break;
        }
        case Interface::FITKOFSolverTypeEnum::RANS:{
            _ui->radioButton_RANS->setChecked(true);
            break;
        }
        case Interface::FITKOFSolverTypeEnum::LES:{
            _ui->radioButton_LES->setChecked(true);
            break;
        }
        }

        updateSubWidget();
    }

    void TurbulenceWidget::slotRadioButtonClicked()
    {
        if (_turManager == nullptr)return;
        if (_physicsData == nullptr)return;
        if (_factoryData == nullptr)return;
        auto solverData = _physicsData->getSolver();
        if (solverData == nullptr)return;

        Interface::FITKOFSolverTypeEnum::FITKOFTurbulenceType type;
        if (_radioGroup->checkedButton() == _ui->radioButton_Laminar) {
            _ui->widget_sub->hide();
            type = Interface::FITKOFSolverTypeEnum::TurbNone;
            _factoryData->setTurbence("");
        }
        else if (_radioGroup->checkedButton() == _ui->radioButton_RANS) {
            _ui->widget_sub->show();
            _ui->widget_Model->show();
            _ui->widget_Delta->hide();
            type = Interface::FITKOFSolverTypeEnum::RANS;
            QStringList modelTypes = _turManager->filterTurbence(type, solverData->getSolverType());
            _ui->comboBox_Model->clear();
            _ui->comboBox_Model->addItems(modelTypes);
            _ui->comboBox_Model->setCurrentIndex(0);

            _factoryData->setTurbence(_ui->comboBox_Model->currentText());
        }
        else if (_radioGroup->checkedButton() == _ui->radioButton_LES) {
            _ui->widget_sub->show();
            _ui->widget_Model->show();
            _ui->widget_Delta->show();
            type = Interface::FITKOFSolverTypeEnum::LES;
            QStringList modelTypes = _turManager->filterTurbence(type, solverData->getSolverType());
            _ui->comboBox_Model->clear();
            _ui->comboBox_Model->addItems(modelTypes);
            _ui->comboBox_Model->setCurrentIndex(0);

            _factoryData->setTurbence(_ui->comboBox_Model->currentText());
        }

        updateSubWidget();
    }

    void TurbulenceWidget::slotComboBoxModelChange(int index)
    {
        if (index == -1)return;
        if (_factoryData == nullptr)return;
        _factoryData->setTurbence(_ui->comboBox_Model->currentText());
        updateSubWidget();
    }

    void TurbulenceWidget::slotComboBoxDeltaChange(int index)
    {
        if (index == -1)return;
        //_turData->setCurrentDeltaType(_ui->comboBox_Delta->currentText());
        //updateSubWidget();
    }

    void TurbulenceWidget::on_pushButton_ModelUnfold_clicked()
    {
        if (_ui->pushButton_ModelUnfold->isChecked()) {
            _ui->widget_ModelSub->show();
        }
        else {
            _ui->widget_ModelSub->hide();
        }

        updateSubWidget();
    }

    void TurbulenceWidget::on_pushButton_DeltaUnfold_clicked()
    {
        if (_ui->pushButton_DeltaUnfold->isChecked()) {
            _ui->widget_DeltaSub->show();
        }
        else {
            _ui->widget_DeltaSub->hide();
        }

        updateSubWidget();
    }
}
