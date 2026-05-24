/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshBaseWidget.h"
#include "ui_MeshBaseWidget.h"
#include "MeshBaseTypeBoxWidget.h"
#include "MeshBaseTypeCylinderWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

namespace GUI
{
    MeshBaseWidget::MeshBaseWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::MeshBaseWidget();
        _ui->setupUi(this);
        _meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();

        init();
    }

    MeshBaseWidget::~MeshBaseWidget()
    {
        if (_ui)delete _ui;
        if (_subWidget)delete _subWidget;
    }

    void MeshBaseWidget::init()
    {
        //初始化type
        _ui->comboBox_Type->addItem(tr("Box"), Interface::FITKAbstractRegionMeshSize::RegionType::RegionBox);
        _ui->comboBox_Type->addItem(tr("Cylinder"), Interface::FITKAbstractRegionMeshSize::RegionType::RegionCylinder);

        //默认类型为box
        Interface::FITKAbstractRegionMeshSize::RegionType type = Interface::FITKAbstractRegionMeshSize::RegionBox;

        //获取第一位数据
        _currentObj = _meshSizeManager->getDataByIndex(0);
        if (!_currentObj) {
            auto meshGenerator = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
            _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionBox);
            _meshSizeManager->insertDataObj(0, _currentObj);
        }
        else {
            switch (_currentObj->getRegionType()) {
            case Interface::FITKAbstractRegionMeshSize::RegionCylinder: break;
            case Interface::FITKAbstractRegionMeshSize::RegionBox: break;
            case Interface::FITKAbstractRegionMeshSize::RegionSphere:
            case Interface::FITKAbstractRegionMeshSize::RegionNone:
            case Interface::FITKAbstractRegionMeshSize::RigonFromFile:
            case Interface::FITKAbstractRegionMeshSize::RigonGeom:
            case Interface::FITKAbstractRegionMeshSize::RegionUserDef1:
            case Interface::FITKAbstractRegionMeshSize::RegionUserDef2:
            case Interface::FITKAbstractRegionMeshSize::RegionUserDef3:
            case Interface::FITKAbstractRegionMeshSize::RegionUserDef4:
            case Interface::FITKAbstractRegionMeshSize::RegionUserDef5: {
                auto meshGenerator = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionBox);
                _meshSizeManager->insertDataObj(0, _currentObj);
            }
            }
        }

        type = _currentObj->getRegionType();

        switch (type) {
        case Interface::FITKAbstractRegionMeshSize::RegionBox: _subWidget = new MeshBaseTypeBoxWidget(this); break;
        case Interface::FITKAbstractRegionMeshSize::RegionCylinder:_subWidget = new MeshBaseTypeCylinderWidget(this); break;
        case Interface::FITKAbstractRegionMeshSize::RegionSphere:break;
        }

        if (_subWidget == nullptr)return;
        _ui->comboBox_Type->setCurrentIndex(_ui->comboBox_Type->findData(type));

        _subWidget->setDataToWidget(_currentObj);
        _subWidget->updateGeometryGraph();
        _ui->gridLayout_SubWidget->addWidget(_subWidget);
    }

    void MeshBaseWidget::saveValue()
    {
        if (_subWidget == nullptr)return;
        _subWidget->getDataFromWidget(_currentObj);
    }

    void MeshBaseWidget::on_comboBox_Type_activated(int index)
    {
        Q_UNUSED(index);
        Interface::FITKAbstractRegionMeshSize::RegionType type = _ui->comboBox_Type->currentData().value<Interface::FITKAbstractRegionMeshSize::RegionType>();
        auto meshGenerator = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();

        switch (type) {
        case Interface::FITKAbstractRegionMeshSize::RegionBox: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionBox);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
                //移除对象但不释放内存
                _meshSizeManager->removeDataObjWithoutRelease(_currentObj);
            }
            else
            {
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionBox);
            }
            //插入到首位
            _meshSizeManager->insertDataObj(0, _currentObj);
            //重新添加界面
            updateWidget(new MeshBaseTypeBoxWidget(this));
            break;
        }
        case Interface::FITKAbstractRegionMeshSize::RegionCylinder: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionCylinder);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
                //移除对象但不释放内存
                _meshSizeManager->removeDataObjWithoutRelease(_currentObj);
            }
            else
            {
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionCylinder);
            }
            //插入到首位
            _meshSizeManager->insertDataObj(0, _currentObj);
            //重新添加界面
            updateWidget(new MeshBaseTypeCylinderWidget(this));
            break;
        }
        case Interface::FITKAbstractRegionMeshSize::RegionSphere: {
            QList<Interface::FITKAbstractRegionMeshSize*> meshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RegionSphere);
            if (meshSizeList.size() != 0) {
                _currentObj = meshSizeList[0];
                //移除对象但不释放内存
                _meshSizeManager->removeDataObjWithoutRelease(_currentObj);
            }
            else
            {
                _currentObj = meshGenerator->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionSphere);
            }
            break;
        }
        }
    }

    void MeshBaseWidget::updateWidget(MeshBaseTypeWidgetBase* newWidget)
    {
        if (newWidget == nullptr)return;
        //重新添加界面
        if (_subWidget) {
            _ui->gridLayout_SubWidget->removeWidget(_subWidget);
            delete _subWidget;
            _subWidget = nullptr;
        }
        _subWidget = newWidget;
        _subWidget->setDataToWidget(_currentObj);
        _subWidget->updateGeometryGraph();
        _ui->gridLayout_SubWidget->addWidget(_subWidget);
    }
}


