/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "LocalGroupDeleteDialog.h"
#include "ui_MeshDeleteDialog.h"
#include "LocalGroupInfoWidget.h"
#include "LocalSelectGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"

namespace GUI
{
    LocalGroupDeleteDialog::LocalGroupDeleteDialog(Interface::FITKGeometryMeshSize* obj, EventOper::ParaWidgetInterfaceOperator* oper) :
        GUIDialogBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper), _obj(obj)
    {
        _ui = new Ui::MeshDeleteDialog();
        _ui->setupUi(this);

        if (_obj) {
            QString text = tr("Delete %1").arg(_obj->getDataObjectName());
            _ui->label_Name->setText(text);
        }
    }

    LocalGroupDeleteDialog::~LocalGroupDeleteDialog()
    {
        if (_ui)delete _ui;
    }

    void LocalGroupDeleteDialog::on_pushButton_Cancel_clicked()
    {
        if (_oper == nullptr)return;
        _oper->execProfession();

        this->reject();
    }

    void LocalGroupDeleteDialog::on_pushButton_OK_clicked()
    {
        if (_obj == nullptr)return;
        if (_oper == nullptr)return;

        Interface::FITKMeshGenInterface* instance = Interface::FITKMeshGenInterface::getInstance();
        Interface::FITKGeometryMeshSizeManager* manager = instance->getGeometryMeshSizeManager();
        if (manager == nullptr)return;

        //如果清除的是当前页面数据对象，清除界面
        LocalGroupInfoWidget* widget = dynamic_cast<LocalGroupInfoWidget*>(_mainWin->getPropertyWidget()->getCurrentWidget());
        if (widget) {
            Interface::FITKGeometryMeshSize* curGeoMeshSize = widget->getCurrentGeoMeshObj();
            if (_obj == curGeoMeshSize) {
                _mainWin->getPropertyWidget()->init();
            }
        }

        //移除数据
        manager->removeDataByID(_obj->getDataObjectID());

        //面组选择界面存在刷新选择参数
        auto localSelectWidget = dynamic_cast<LocalSelectGroupWidget*>(_mainWin->getPropertyWidget()->getCurrentWidget());
        if (localSelectWidget) {
            localSelectWidget->updateTableWidget();
        }

        _oper->execProfession();

        this->accept();
    }
}

