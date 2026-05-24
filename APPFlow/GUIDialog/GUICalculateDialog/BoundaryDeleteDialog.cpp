/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "BoundaryDeleteDialog.h"
#include "ui_BoundaryDeleteDialog.h"
#include "BoundaryWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundaryTypeManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"

namespace GUI 
{
    BoundaryDeleteDialog::BoundaryDeleteDialog(Interface::FITKOFBoundary* obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIDialogBase(FITKAPP->getGlobalData()->getMainWindow()), _obj(obj), _oper(oper)
    {
        _ui = new Ui::BoundaryDeleteDialog();
        _ui->setupUi(this);

        if (_obj) {
            QString text = tr("Delete %1").arg(_obj->getDataObjectName());
            _ui->label_Name->setText(text);
        }

        setWindowTitle(tr("Boundary Condition Delete"));
    }

    BoundaryDeleteDialog::~BoundaryDeleteDialog()
    {
        if (_ui)delete _ui;
    }

    void BoundaryDeleteDialog::on_pushButton_OK_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKOFPhysicsData* physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (physicsData == nullptr)return;
        Interface::FITKOFBoundaryManager* manager = physicsData->getBoundaryManager();
        if (manager == nullptr)return;
        GUI::MainWindow* mainWin = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWin == nullptr)return;
        auto propertyWidget = mainWin->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        //清除当前数据对应的界面
        BoundaryWidget* boundWidget = dynamic_cast<BoundaryWidget*>(propertyWidget->getCurrentWidget());
        if (boundWidget) {
            if (_obj == boundWidget->getCurrentObj()) {
                propertyWidget->init();
            }
        }

        //清除数据
        manager->removeDataByID(_obj->getDataObjectID());

        //执行操作器进行刷新
        if (_oper) {
            _oper->execProfession();
        }
        this->accept();
    }

    void BoundaryDeleteDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }
}

