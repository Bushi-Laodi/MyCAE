/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshGeoSubWidget.h"
#include "ui_MeshGeoSubWidget.h"

#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionGeometryRefine.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"

namespace GUI
{
    MeshGeoSubWidget::MeshGeoSubWidget(Interface::FITKAbsGeoCommand* geoObj, EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _geoObj(geoObj), _oper(oper)
    {
        _ui = new Ui::MeshGeoSubWidget();
        _ui->setupUi(this);

       init();
    }


    MeshGeoSubWidget::~MeshGeoSubWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    int MeshGeoSubWidget::getObjID()
    {
        if (_geoObj == nullptr)return -1;
        return _geoObj->getDataObjectID();
    }

    void MeshGeoSubWidget::init()
    {
        //管理器获取
        _meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        _geoRefineManager = Interface::FITKMeshGenInterface::getInstance()->getRegionGeometryRefineManager();

        //阻断信号
        QSignalBlocker blocker(_ui->comboBox_RefineModel);
        QSignalBlocker blocker1(_ui->comboBox_Gap);
        _ui->comboBox_RefineModel->addItem(tr("Distance"),Interface::FITKRegionGeometryRefine::RefineModelType::RefineDistance);
        _ui->comboBox_RefineModel->addItem(tr("Inside"),Interface::FITKRegionGeometryRefine::RefineModelType::RefineInside);
        _ui->comboBox_RefineModel->addItem(tr("Outside"),Interface::FITKRegionGeometryRefine::RefineModelType::RefineOutside);
        _ui->comboBox_Gap->addItem(tr("none"), Interface::FITKRegionGeometryRefine::GapModelType::GapNone);
        _ui->comboBox_Gap->addItem(tr("Mixed"), Interface::FITKRegionGeometryRefine::GapModelType::GapMixed);
        _ui->comboBox_Gap->addItem(tr("Inside"), Interface::FITKRegionGeometryRefine::GapModelType::GapInside);
        _ui->comboBox_Gap->addItem(tr("Outside"), Interface::FITKRegionGeometryRefine::GapModelType::GapOutside);
        _ui->comboBox_RefineModel->setCurrentIndex(0);
        _ui->comboBox_Gap->setCurrentIndex(0);

        setDataToWidget();
        updateWidget();
    }

    void MeshGeoSubWidget::updateWidget()
    {
        if (_geoMeshSize) {
            _ui->checkBox_Mesh->setChecked(true);
            _ui->groupBox_Mesh->show();
        }
        else {
            _ui->checkBox_Mesh->setChecked(false);
            _ui->groupBox_Mesh->hide();
        }

        if (_geoRefine) {
            _ui->checkBox_Refine->setChecked(true);
            _ui->groupBox_Refine->show();

            switch (_ui->comboBox_RefineModel->currentData().value<Interface::FITKRegionGeometryRefine::RefineModelType>()){
            case Interface::FITKRegionGeometryRefine::RefineDistance: {
                _ui->label_Distance->show();
                _ui->doubleSpinBox_Distance->show();
                break;
            }
            case Interface::FITKRegionGeometryRefine::RefineInside:
            case Interface::FITKRegionGeometryRefine::RefineOutside:{
                _ui->label_Distance->hide();
                _ui->doubleSpinBox_Distance->hide();
                break;
            }
            }

            switch (_ui->comboBox_Gap->currentData().value<Interface::FITKRegionGeometryRefine::GapModelType>()) {
            case Interface::FITKRegionGeometryRefine::GapNone: {
                _ui->label_NoCell->hide();
                _ui->spinBox_NoCell->hide();
                _ui->label_MaxGap->hide();
                _ui->spinBox_MaxGap->hide();
                break;
            }
            case Interface::FITKRegionGeometryRefine::GapMixed:
            case Interface::FITKRegionGeometryRefine::GapInside:
            case Interface::FITKRegionGeometryRefine::GapOutside: {
                _ui->label_NoCell->show();
                _ui->spinBox_NoCell->show();
                _ui->label_MaxGap->show();
                _ui->spinBox_MaxGap->show();
                break;
            }
            }
        }
        else {
            _ui->checkBox_Refine->setChecked(false);
            _ui->groupBox_Refine->hide();
        }
    }

    void MeshGeoSubWidget::on_spinBox_Min_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::on_spinBox_Max_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::on_checkBox_Mesh_clicked()
    {
        if (_geoObj == nullptr)return;
        if (_meshSizeManager == nullptr)return;
        if (_ui->checkBox_Mesh->isChecked()) {
            auto meshSizeGen = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
            if (meshSizeGen) {
                _geoMeshSize = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>
                    (meshSizeGen->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom));
                if (_geoMeshSize) {
                    _geoMeshSize->setGeomID(_geoObj->getDataObjectID());
                    _meshSizeManager->appendDataObj(_geoMeshSize);
                }
            }
        }
        else {
            if (_geoMeshSize) {
                _meshSizeManager->removeDataByID(_geoMeshSize->getDataObjectID());
                _geoMeshSize = nullptr;
            }
        }
        updateWidget();
    }

    void MeshGeoSubWidget::on_checkBox_Refine_clicked()
    {
        if (_geoObj == nullptr)return;
        if (_geoRefineManager == nullptr)return;
        if (_ui->checkBox_Refine->isChecked()) {
            _geoRefine = new Interface::FITKRegionGeometryRefine();
            _geoRefine->setGeometryId(_geoObj->getDataObjectID());
            _geoRefineManager->appendDataObj(_geoRefine);
        }
        else {
            if (_geoRefine) {
                _geoRefineManager->removeDataByID(_geoRefine->getDataObjectID());
                _geoRefine = nullptr;
            }
        }
        updateWidget();
    }

    void MeshGeoSubWidget::on_checkBox_Limit_clicked()
    {
        getDataFromWidget();
    }

    void GUI::MeshGeoSubWidget::on_comboBox_RefineModel_currentIndexChanged(int index)
    {
        Q_UNUSED(index);
        getDataFromWidget();
        updateWidget();
    }

    void MeshGeoSubWidget::on_doubleSpinBox_Distance_valueChanged(double arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::on_spinBox_Level_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::on_comboBox_Gap_currentIndexChanged(int index)
    {
        Q_UNUSED(index);
        getDataFromWidget();
        updateWidget();
    }

    void MeshGeoSubWidget::on_spinBox_NoCell_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::on_spinBox_MaxGap_valueChanged(int arg1)
    {
        Q_UNUSED(arg1);
        getDataFromWidget();
    }

    void MeshGeoSubWidget::setDataToWidget()
    {
        if (_geoObj == nullptr)return;
        if (_meshSizeManager ==nullptr)return;
        if (_geoRefineManager == nullptr)return;

        //网格划分区域尺寸对象获取
        auto geoMeshSizeList = _meshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom);
        for (int i = 0; i < geoMeshSizeList.size(); i++) {
            Interface::FITKRegionMeshSizeGeom* geoMeshSize = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>(geoMeshSizeList[i]);
            if (geoMeshSize == nullptr)continue;
            if (geoMeshSize->getGeomID() == _geoObj->getDataObjectID()) {
                _geoMeshSize = geoMeshSize;
                break;
            }
        }
        //网格划分优化对象获取
        for (int i = 0; i < _geoRefineManager->getDataCount(); i++) {
            auto geoRefineObj = _geoRefineManager->getDataByIndex(i);
            if (geoRefineObj == nullptr)continue;
            if (geoRefineObj->getGeometryId() == _geoObj->getDataObjectID())_geoRefine = geoRefineObj;
        }

        //阻断信号
        QSignalBlocker blocker(_ui->spinBox_Max);
        QSignalBlocker blocker1(_ui->spinBox_Min);
        QSignalBlocker blocker2(_ui->checkBox_Limit);
        QSignalBlocker blocker3(_ui->comboBox_RefineModel);
        QSignalBlocker blocker4(_ui->doubleSpinBox_Distance);
        QSignalBlocker blocker5(_ui->spinBox_Level);
        QSignalBlocker blocker6(_ui->comboBox_Gap);
        QSignalBlocker blocker7(_ui->spinBox_NoCell);
        QSignalBlocker blocker8(_ui->spinBox_MaxGap);

        //网格划分尺寸数据填充
        if (_geoMeshSize) {
            _ui->spinBox_Max->setValue(_geoMeshSize->getMaxSize());
            _ui->spinBox_Min->setValue(_geoMeshSize->getMinSize());
        }
        //网格划分优化数据填充
        if (_geoRefine) {
            _ui->checkBox_Limit->setChecked(_geoRefine->getLimitRefineMent());
            _ui->comboBox_RefineModel->setCurrentIndex(_ui->comboBox_RefineModel->findData(_geoRefine->getRefineModel()));
            _ui->doubleSpinBox_Distance->setValue(_geoRefine->getDistance());
            _ui->spinBox_Level->setValue(_geoRefine->getLevel());
            _ui->comboBox_Gap->setCurrentIndex(_ui->comboBox_Gap->findData(_geoRefine->getGapRefineModel()));
            _ui->spinBox_NoCell->setValue(_geoRefine->getCellsInGapNum());
            _ui->spinBox_MaxGap->setValue(_geoRefine->getMaxGapLevel());
        }
    }

    void MeshGeoSubWidget::getDataFromWidget()
    {
        if (_geoMeshSize) {
            _geoMeshSize->setMaxSize(_ui->spinBox_Max->value());
            _geoMeshSize->setMinSize(_ui->spinBox_Min->value());
        }

        if (_geoRefine) {
            _geoRefine->setLimitRefinement(_ui->checkBox_Limit->isChecked());
            _geoRefine->setRefineModel(_ui->comboBox_RefineModel->currentData().value<Interface::FITKRegionGeometryRefine::RefineModelType>());
            _geoRefine->setDistance(_ui->doubleSpinBox_Distance->value());
            _geoRefine->setLevel(_ui->spinBox_Level->value());
            _geoRefine->setGapRefineModel(_ui->comboBox_Gap->currentData().value<Interface::FITKRegionGeometryRefine::GapModelType>());
            _geoRefine->setCellsInGapNum(_ui->spinBox_NoCell->value());
            _geoRefine->setMaxGapLevel(_ui->spinBox_MaxGap->value());
        }
    }
}

