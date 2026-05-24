/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "InitialCreatePatchDialog.h"
#include "ui_InitialCreatePatchDialog.h"
#include "InitialWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFInitialConditions.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"

#include <QButtonGroup>
#include <QCheckBox>

#define InitialApplyPos "InitialApplyPos"

namespace GUI {
    InitialCreatePatchDialog::InitialCreatePatchDialog(QWidget* parent) :
        GUIDialogBase(parent)
    {
        _parentWidget = dynamic_cast<InitialWidget*>(parent);
        _ui = new Ui::InitialCreatePatchDialog();
        _ui->setupUi(this);
        init();
        this->setWindowTitle("Patch Dialog");
    }

    InitialCreatePatchDialog::~InitialCreatePatchDialog()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void InitialCreatePatchDialog::init()
    {
        _physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        _factoryData = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (_physicsData == nullptr)return;
        int patchCount = _physicsData->getInitialConditions()->getPatchCount();
        //几何数据填充
        QList<int> pathGeoIDs = {};
        for (int j = 0; j < patchCount; j++) {
            auto patchData = _physicsData->getInitialConditions()->getPatch(j);
            if (patchData == nullptr)continue;
            pathGeoIDs.append(patchData->getGeometryId());
        }
        Interface::FITKGeoCommandList* geoList = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geoList == nullptr) return;
        for (int i = 0; i < geoList->getDataCount(); i++) {
            auto geo = geoList->getDataByIndex(i);
            if (geo == nullptr)continue;
            if (pathGeoIDs.contains(geo->getDataObjectID()))continue;
            _ui->comboBox_Geometry->addItem(geo->getDataObjectName(), geo->getDataObjectID());
        }

        //场数据填充类型填充
        QStringList fields = _factoryData->filterPatchFieldsType();
        for (QString filed : fields) {
            QCheckBox* checkBox = new QCheckBox(filed);
            _ui->verticalLayout_Fields->addWidget(checkBox);
            _fields.append(checkBox);
        }

        //应用类型初始化
        _radioGroup = new QButtonGroup(this);
        _ui->radioButton_Cell->setProperty(InitialApplyPos, Interface::FITKOFSolverInitialEnum::FITKPatchApplyObjectType::Cells);
        _ui->radioButton_Face->setProperty(InitialApplyPos, Interface::FITKOFSolverInitialEnum::FITKPatchApplyObjectType::Faces);
        _ui->radioButton_Both->setProperty(InitialApplyPos, Interface::FITKOFSolverInitialEnum::FITKPatchApplyObjectType::Both);
        _radioGroup->addButton(_ui->radioButton_Cell);
        _radioGroup->addButton(_ui->radioButton_Face);
        _radioGroup->addButton(_ui->radioButton_Both);
    }

    void InitialCreatePatchDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }

    void InitialCreatePatchDialog::on_pushButton_OK_clicked()
    {
        if (_factoryData == nullptr)return;
        int geoId = -1;
        geoId = _ui->comboBox_Geometry->currentData().toInt();
        if (geoId <= 0)return;

        QAbstractButton* radioButton = _radioGroup->checkedButton();
        auto type = radioButton->property(InitialApplyPos).value<Interface::FITKOFSolverInitialEnum::FITKPatchApplyObjectType>();

        QStringList fileds = {};
        for (auto c : _fields) {
            if(c == nullptr)continue;
            if (c->isChecked()) {
                fileds.append(c->text());
            }
        }
        _factoryData->setPatch(geoId, fileds, type);
        this->accept();
    }
}
