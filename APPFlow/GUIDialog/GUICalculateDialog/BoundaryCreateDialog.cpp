/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "BoundaryCreateDialog.h"
#include "ui_BoundaryCreateDialog.h"

#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

//类型注册
Q_DECLARE_METATYPE(Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType)

namespace GUI
{
    BoundaryCreateDialog::BoundaryCreateDialog(EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIDialogBase(FITKAPP->getGlobalData()->getMainWindow()), _oper(oper)
    {
        _ui = new Ui::BoundaryCreateDialog();
        _ui->setupUi(this);

        _physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        _meshData = FITKAPP->getGlobalData()->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        _factoryData = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        init();

        this->setWindowTitle(tr("Create Boundary"));
    }

    BoundaryCreateDialog::~BoundaryCreateDialog()
    {
        if (_ui)delete _ui;
    }

    void BoundaryCreateDialog::init()
    {
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //获取边界属性数据管理器
        Interface::FITKOFBoundaryManager* boundaryManager = _physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return;

        //界面名称添加
        _ui->lineEdit_Name->setText(boundaryManager->checkName("Boundary-1"));

        //初始化网格区域下拉框
        int count = _meshData->getDataCount();
        for (int i = 0; i < count; ++i)
        {
            Interface::FITKFluidRegionsMesh* region = _meshData->getDataByIndex(i);
            if (!region || _physicsData->getRegionMeshType(region->getDataObjectID()) == Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::None) continue;
            _ui->comboBox_MeshRegion->addItem(region->getDataObjectName(), region->getDataObjectID());
        }
        //初始化边界域下拉框
        for (int i = 0; i < _ui->comboBox_MeshRegion->count(); ++i)
        {
            _ui->comboBox_MeshRegion->setCurrentIndex(i);
            this->initBoundaryComboBox();
            if (_ui->comboBox_Boundary->count() > 0)
                break;
        }
        this->initInterfaceMeshRegionComboBox();
        this->initInterfaceBoundaryComboBox();
        
        //边界类型添加
        _ui->comboBox_Type->addItem(tr("Wall"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWall);
        _ui->comboBox_Type->addItem(tr("Pressure Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureInlet);
        _ui->comboBox_Type->addItem(tr("Velocity Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BVelocityInlet);
        _ui->comboBox_Type->addItem(tr("Pressure Outlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureOutlet);
        _ui->comboBox_Type->addItem(tr("Outflow"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BOutflow);
        _ui->comboBox_Type->addItem(tr("Symmetry"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BSymmetry);
        _ui->comboBox_Type->addItem(tr("Wedge"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWedge);
        _ui->comboBox_Type->addItem(tr("Mapped Wall"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BMappedWall);

        if (_ui->comboBox_Type->currentData().toInt() != Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BMappedWall)
        {
            _ui->comboBox_InterMeshRegion->setEnabled(false);
            _ui->comboBox_InterBoundary->setEnabled(false);
        }
    }

    void BoundaryCreateDialog::hideEvent(QHideEvent * event)
    {
        GUIDialogBase::hideEvent(event);
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
    }

    void BoundaryCreateDialog::showEvent(QShowEvent * event)
    {
        GUIDialogBase::showEvent(event);
        int id = -1;
        if (_ui->comboBox_Boundary->count() > 0) {
            id = _ui->comboBox_Boundary->currentData().toInt();
        }
        highlightMeshBoundary(id);
    }

    void BoundaryCreateDialog::on_comboBox_Boundary_activated(int index)
    {
        Q_UNUSED(index);
        highlightMeshBoundary(_ui->comboBox_Boundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_comboBox_MeshRegion_activated(int index)
    {
        Q_UNUSED(index);
        //更新ComboBox_boundary组件
        this->initBoundaryComboBox();
        this->initInterfaceMeshRegionComboBox();
        this->initInterfaceBoundaryComboBox();
        if (_ui->comboBox_Boundary->count() > 0)
            highlightMeshBoundary(_ui->comboBox_Boundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_comboBox_InterBoundary_activated(int index)
    {
        Q_UNUSED(index);
        highlightMeshBoundary(_ui->comboBox_InterBoundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_comboBox_InterMeshRegion_activated(int index)
    {
        Q_UNUSED(index);
        //更新ComboBox_boundary组件
        this->initInterfaceBoundaryComboBox();
        if (_ui->comboBox_InterBoundary->count() > 0)
            highlightMeshBoundary(_ui->comboBox_Boundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_comboBox_Type_activated(int index)
    {
        Q_UNUSED(index);
        if (_ui->comboBox_Type->currentData().toInt() != Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BMappedWall)
        {
            _ui->comboBox_InterMeshRegion->setEnabled(false);
            _ui->comboBox_InterBoundary->setEnabled(false);
        }
        else
        {
            _ui->comboBox_InterMeshRegion->setEnabled(true);
            _ui->comboBox_InterBoundary->setEnabled(true);
        }
    }

    void BoundaryCreateDialog::on_pushButton_OK_clicked()
    {
        if (_ui->comboBox_Boundary->count() == 0 || _physicsData == nullptr || _factoryData == nullptr || _meshData == nullptr)return;
        //获取边界管理器
        auto boundManager = _physicsData->getBoundaryManager();
        if (boundManager == nullptr)return;
        //获取边界网格管理器
        int regionMeshID = _ui->comboBox_MeshRegion->currentData().toInt();
        int meshBoundaryID = _ui->comboBox_Boundary->currentData().toInt();
        int regionMeshID2 = _ui->comboBox_InterMeshRegion->currentData().toInt();
        int meshBoundaryID2 = _ui->comboBox_InterBoundary->currentData().toInt();
        Interface::FITKFluidRegionsMesh* region = _meshData->getDataByID(regionMeshID);//获取区域数据
        if (!region) return;
        Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType type = _ui->comboBox_Type->currentData().value<Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType>();
        //获取区域类型
        QString name = _ui->lineEdit_Name->text();
        if (type == Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BMappedWall)
            _factoryData->setMappedWallBoundary(regionMeshID, meshBoundaryID, regionMeshID2, meshBoundaryID2);
        else
            _factoryData->setBoundary(regionMeshID, meshBoundaryID, type, _physicsData->getRegionMeshType(region->getDataObjectID()));
        auto boundary = boundManager->getBoundary(meshBoundaryID);
        auto boundary2 = boundManager->getBoundary(meshBoundaryID2);
        if (boundary) {
            boundary->setDataObjectName(name);
        }
        if (boundary2) {
            boundary2->setDataObjectName(QString("%1-Mapped").arg(name));
        }
        
        if (_oper) {
            _oper->execProfession();
        }

        this->accept();
    }

    void BoundaryCreateDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }

    void BoundaryCreateDialog::initBoundaryComboBox()
    {
        _ui->comboBox_Boundary->clear();
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //获取已存在的边界属性数据管理器
        Interface::FITKOFBoundaryManager* boundaryManager = _physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return;
        //获取边界网格管理器
        int regionMeshID = _ui->comboBox_MeshRegion->currentData().toInt();
        Interface::FITKFluidRegionsMesh* region = _meshData->getDataByID(regionMeshID);//获取区域数据
        if (!region) return;
        Interface::FITKBoundaryMeshVTKManager* boundMeshManager = region->getBoundaryMeshManager();
        if (boundMeshManager == nullptr)return;
        //初始化边界下拉框
        for (int i = 0; i < boundMeshManager->getDataCount(); i++) {
            Interface::FITKBoundaryMeshVTK* boundMesh = boundMeshManager->getDataByIndex(i);
            if (boundMesh == nullptr)continue;
            if (boundaryManager->getBoundary(boundMesh->getDataObjectID()))continue;
            _ui->comboBox_Boundary->addItem(boundMesh->getDataObjectName(), boundMesh->getDataObjectID());
        }
        _ui->comboBox_Boundary->setCurrentIndex(0);
    }

    void BoundaryCreateDialog::initInterfaceBoundaryComboBox()
    {
        _ui->comboBox_InterBoundary->clear();
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //获取已存在的边界属性数据管理器
        Interface::FITKOFBoundaryManager* boundaryManager = _physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return;
        //获取边界网格管理器
        int regionMeshID = _ui->comboBox_InterMeshRegion->currentData().toInt();
        Interface::FITKFluidRegionsMesh* region = _meshData->getDataByID(regionMeshID);//获取区域数据
        if (!region) return;
        Interface::FITKBoundaryMeshVTKManager* boundMeshManager = region->getBoundaryMeshManager();
        if (boundMeshManager == nullptr)return;
        //初始化边界下拉框
        for (int i = 0; i < boundMeshManager->getDataCount(); i++) {
            Interface::FITKBoundaryMeshVTK* boundMesh = boundMeshManager->getDataByIndex(i);
            if (boundMesh == nullptr)continue;
            if (boundaryManager->getBoundary(boundMesh->getDataObjectID()))continue;
            _ui->comboBox_InterBoundary->addItem(boundMesh->getDataObjectName(), boundMesh->getDataObjectID());
        }
        _ui->comboBox_InterBoundary->setCurrentIndex(0);
    }

    void BoundaryCreateDialog::initInterfaceMeshRegionComboBox()
    {
        _ui->comboBox_InterMeshRegion->clear();
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //初始化网格区域下拉框
        int count = _meshData->getDataCount();
        int regionMeshID = _ui->comboBox_MeshRegion->currentData().toInt();
        for (int i = 0; i < count; ++i)
        {
            Interface::FITKFluidRegionsMesh* region = _meshData->getDataByIndex(i);
            if (!region || _physicsData->getRegionMeshType(region->getDataObjectID()) == Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::None) continue;
            if (regionMeshID == region->getDataObjectID()) continue;
            _ui->comboBox_InterMeshRegion->addItem(region->getDataObjectName(), region->getDataObjectID());
        }
        _ui->comboBox_InterMeshRegion->setCurrentIndex(0);
    }

    void BoundaryCreateDialog::highlightMeshBoundary(int meshBoundID)
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
        graphOper->highlight(meshBoundID);
    }
}