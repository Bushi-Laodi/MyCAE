/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "BoundaryWidget.h"
#include "ui_BoundaryWidget.h"
#include "CompSelectComBoxWidget.h"

#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundaryType.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundaryTypeManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"

#include <QTabBar>
#include <QToolBox>

namespace GUI
{
    /**
     * @brief    FLow子数据回调函数
     * @param[i] type 类型
     * @param[i] widget 对应的界面
     * @return   Core::FITKParameter* 子数据对象
     * @author   BaGuijun (baguijun@163.com)
     * @date     2024-08-27
     */
    Core::FITKParameter* getBoundaryFlowSubData(const QString& type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int objID = widget->getData("objID").toInt();
        QString objName = widget->getData("objName").toString();
        int index = widget->getData("index").toInt();
        int boundaryID = widget->getData("boundaryID").toInt();
        auto boundary = phyData->getBoundaryManager()->getDataByID(boundaryID);
        if (boundary == nullptr)return nullptr;

        phyFactory->setVariableBoundaryType(boundaryID, objName, type);

        return boundary->getFlowVBType(index)->getBoundaryTypePara();
    }
    /**
     * @brief   Turbulence子数据回调函数
     * @param[i] type 类型
     * @param[i] widget 对应的界面
     * @return   Core::FITKParameter* 子数据对象
     * @author   BaGuijun (baguijun@163.com)
     * @date     2024-08-27
     */
    Core::FITKParameter* getBoundaryTurbulenceSubData(const QString& type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int objID = widget->getData("objID").toInt();
        QString objName = widget->getData("objName").toString();
        int index = widget->getData("index").toInt();
        int boundaryID = widget->getData("boundaryID").toInt();
        auto boundary = phyData->getBoundaryManager()->getDataByID(boundaryID);
        if (boundary == nullptr)return nullptr;

        phyFactory->setVariableBoundaryType(boundaryID, objName, type);
        return boundary->getTurbulenceVBType(index)->getBoundaryTypePara();
    }
    /**
     * @brief    Phases子数据回调函数
     * @param[i] type 类型
     * @param[i] widget 对应的界面
     * @return   Core::FITKParameter* 子数据对象
     * @author   BaGuijun (baguijun@163.com)
     * @date     2024-08-27
     */
    Core::FITKParameter* getBoundaryPhasesSubData(const QString& type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int objID = widget->getData("objID").toInt();
        QString objName = widget->getData("objName").toString();
        int index = widget->getData("index").toInt();
        int boundaryID = widget->getData("boundaryID").toInt();
        auto boundary = phyData->getBoundaryManager()->getDataByID(boundaryID);
        if (boundary == nullptr)return nullptr;

        phyFactory->setVariableBoundaryType(boundaryID, objName, type);
        return boundary->getPhasesVBType(index)->getBoundaryTypePara();
    }
    /**
     * @brief    Thermal子数据回调函数
     * @param[i] type 类型
     * @param[i] widget 对应的界面
     * @return   Core::FITKParameter* 子数据对象
     * @author   BaGuijun (baguijun@163.com)
     * @date     2024-08-27
     */
    Core::FITKParameter* getBoundaryThermalSubData(const QString& type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int objID = widget->getData("objID").toInt();
        QString objName = widget->getData("objName").toString();
        int index = widget->getData("index").toInt();
        int boundaryID = widget->getData("boundaryID").toInt();
        auto boundary = phyData->getBoundaryManager()->getDataByID(boundaryID);
        if (boundary == nullptr)return nullptr;

        phyFactory->setVariableBoundaryType(boundaryID, objName, type);
        return boundary->getThermalVBType(index)->getBoundaryTypePara();
    }

    BoundaryWidget::BoundaryWidget(Interface::FITKOFBoundary* boundaryObj, EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent), _boundaryObj(boundaryObj)
    {
        _ui = new Ui::BoundaryWidget();
        _ui->setupUi(this);
        init();
    }

    BoundaryWidget::~BoundaryWidget()
    {
        if (_ui) delete _ui;
    }

    void BoundaryWidget::init()
    {
        if (_boundaryObj == nullptr)return;

        //名称设置
        QString name = _boundaryObj->getDataObjectName();
        _ui->lineEdit_Name->setText(name);
        _ui->lineEdit_Name->setEnabled(false);

        //网格边界名称设置
        auto meshBoundary = _boundaryObj->getMeshBoundary();
        _ui->lineEdit_Boundary->setEnabled(false);
        if (meshBoundary)_ui->lineEdit_Boundary->setText(meshBoundary->getDataObjectName());

        //边界类型名称设置
        _currentType = _boundaryObj->getBoundaryType();
        QString typeName = "";
        switch (_currentType) {
        case Interface::FITKOFSolverTypeEnum::BWall:typeName = tr("Wall"); break;
        case Interface::FITKOFSolverTypeEnum::BPressureInlet:typeName = tr("Pressure Inlet"); break;
        case Interface::FITKOFSolverTypeEnum::BVelocityInlet:typeName = tr("Velocity Inlet"); break;
        case Interface::FITKOFSolverTypeEnum::BPressureOutlet:typeName = tr("Pressure Outlet"); break;
        case Interface::FITKOFSolverTypeEnum::BOutflow:typeName = tr("Outflow"); break;
        case Interface::FITKOFSolverTypeEnum::BSymmetry:typeName = tr("Symmetry"); break;
        case Interface::FITKOFSolverTypeEnum::BWedge:typeName = tr("Wedge"); break;
        case Interface::FITKOFSolverTypeEnum::BMappedWall:typeName = tr("Mapped Wall"); break;
        }
        _ui->lineEdit_Type->setText(typeName);
        _ui->lineEdit_Type->setEnabled(false);

        update();
    }

    void BoundaryWidget::update()
    {
        if (_boundaryObj == nullptr)return;

        updateFlow();
        updateTurbulence();
        updatePhases();
        updateThermal();
    }

    Interface::FITKOFBoundary * BoundaryWidget::getCurrentObj()
    {
        return _boundaryObj;
    }

    void BoundaryWidget::showEvent(QShowEvent * event)
    {
        GUICalculateWidgetBase::showEvent(event);
        if (_boundaryObj == nullptr)return;

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
        graphOper->highlight(_boundaryObj->getMeshBoundaryID());
    }

    void BoundaryWidget::hideEvent(QHideEvent * event)
    {
        GUICalculateWidgetBase::hideEvent(event);
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
    }

    void BoundaryWidget::updateFlow()
    {
        if (_boundaryObj == nullptr)return;
        if (_physicsManager == nullptr)return;
        auto boundartTypeMan = _physicsManager->getBoundaryTypeManager();
        if (boundartTypeMan == nullptr)return;
        int num = _boundaryObj->getFlowCount();
        if (num == 0) {
            _ui->tabWidget->removeTab(0);
            return;
        }

        QWidget* widget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout();
        QToolBox* toolBox = Core::FITKWidgetComLine::CreateToolBox(widget);
        for (int i = 0; i < num; i++) {
            Interface::FITKOFAbsBoundaryType* data = _boundaryObj->getFlowVBType(i);
            if (data == nullptr)continue;
            QStringList options = boundartTypeMan->filterBoundariesType(_physicsData->getSolver()->getSolverType(),
                _currentType, _boundaryObj->getFlowVariableName(i));
            CompSelectComBoxWidget* w = new CompSelectComBoxWidget(_boundaryObj->getFlowVariableName(i), this);
            w->setData("boundaryID", _boundaryObj->getDataObjectID());
            w->setData("objID", data->getDataObjectID());
            w->setData("objName", _boundaryObj->getFlowVariableName(i));
            w->setData("index", i);
            w->setSubWidgetData(data->getBoundaryTypePara());
            w->setOptions(options);
            w->setCurrentText(data->getDataObjectName());
            w->setFunction(&getBoundaryFlowSubData);
            toolBox->addItem(w, _boundaryObj->getFlowVariableName(i));
        }
        layout->addWidget(toolBox);
        widget->setLayout(layout);
        _ui->tabWidget->addTab(widget, tr("Flow"));
    }

    void BoundaryWidget::updateTurbulence()
    {
        if (_boundaryObj == nullptr)return;
        if (_physicsManager == nullptr)return;
        auto boundartTypeMan = _physicsManager->getBoundaryTypeManager();
        if (boundartTypeMan == nullptr)return;
        int num = _boundaryObj->getTurbulenceCount();
        if (num == 0) {
            return;
        }

        QWidget* widget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout();
        QToolBox* toolBox = Core::FITKWidgetComLine::CreateToolBox(widget);
        for (int i = 0; i < num; i++) {
            Interface::FITKOFAbsBoundaryType* data = _boundaryObj->getTurbulenceVBType(i);
            if (data == nullptr)continue;
            QStringList options = boundartTypeMan->filterBoundariesType(_physicsData->getSolver()->getSolverType(),
                _currentType, _boundaryObj->getTurbulenceVariableName(i));
            CompSelectComBoxWidget* w = new CompSelectComBoxWidget(_boundaryObj->getTurbulenceVariableName(i), this);
            w->setData("boundaryID", _boundaryObj->getDataObjectID());
            w->setData("objID", data->getDataObjectID());
            w->setData("objName", _boundaryObj->getTurbulenceVariableName(i));
            w->setData("index", i);
            w->setSubWidgetData(data->getBoundaryTypePara());
            w->setOptions(options);
            w->setCurrentText(data->getDataObjectName());
            w->setFunction(&getBoundaryTurbulenceSubData);
            toolBox->addItem(w, _boundaryObj->getTurbulenceVariableName(i));
        }
        layout->addWidget(toolBox);
        widget->setLayout(layout);
        _ui->tabWidget->addTab(widget, tr("Turbulence"));
    }

    void BoundaryWidget::updatePhases()
    {
        if (_boundaryObj == nullptr)return;
        if (_physicsManager == nullptr)return;
        auto boundartTypeMan = _physicsManager->getBoundaryTypeManager();
        if (boundartTypeMan == nullptr)return;
        int num = _boundaryObj->getPhasesCount();
        if (num == 0) {
            _ui->tabWidget->removeTab(2);
            return;
        }

        QWidget* widget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout();
        QToolBox* toolBox = Core::FITKWidgetComLine::CreateToolBox(widget);
        for (int i = 0; i < num; i++) {
            Interface::FITKOFAbsBoundaryType* data = _boundaryObj->getPhasesVBType(i);
            if (data == nullptr)continue;
            QStringList options = boundartTypeMan->filterBoundariesType(_physicsData->getSolver()->getSolverType(),
                _currentType, _boundaryObj->getPhasesVariableName(i));
            CompSelectComBoxWidget* w = new CompSelectComBoxWidget(_boundaryObj->getPhasesVariableName(i), this);
            w->setData("boundaryID", _boundaryObj->getDataObjectID());
            w->setData("objID", data->getDataObjectID());
            w->setData("objName", _boundaryObj->getPhasesVariableName(i));
            w->setData("index", i);
            w->setSubWidgetData(data->getBoundaryTypePara());
            w->setOptions(options);
            w->setCurrentText(data->getDataObjectName());
            w->setFunction(&getBoundaryPhasesSubData);
            toolBox->addItem(w, _boundaryObj->getPhasesVariableName(i));
        }
        layout->addWidget(toolBox);
        widget->setLayout(layout);
        _ui->tabWidget->addTab(widget, tr("Phases"));
    }

    void BoundaryWidget::updateThermal()
    {
        if (_boundaryObj == nullptr)return;
        if (_physicsManager == nullptr)return;
        auto boundartTypeMan = _physicsManager->getBoundaryTypeManager();
        if (boundartTypeMan == nullptr)return;
        int num = _boundaryObj->getThermalCount();
        if (num == 0) {
            _ui->tabWidget->removeTab(2);
            return;
        }

        QWidget* widget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout();
        QToolBox* toolBox = Core::FITKWidgetComLine::CreateToolBox(widget);
        for (int i = 0; i < num; i++) {
            Interface::FITKOFAbsBoundaryType* data = _boundaryObj->getThermalVBType(i);
            if (data == nullptr)continue;
            QStringList options = boundartTypeMan->filterBoundariesType(_physicsData->getSolver()->getSolverType(),
                _currentType, _boundaryObj->getThermalVariableName(i));
            CompSelectComBoxWidget* w = new CompSelectComBoxWidget(_boundaryObj->getThermalVariableName(i), this);
            w->setData("boundaryID", _boundaryObj->getDataObjectID());
            w->setData("objID", data->getDataObjectID());
            w->setData("objName", _boundaryObj->getThermalVariableName(i));
            w->setData("index", i);
            w->setSubWidgetData(data->getBoundaryTypePara());
            w->setOptions(options);
            w->setCurrentText(data->getDataObjectName());
            w->setFunction(&getBoundaryPhasesSubData);
            toolBox->addItem(w, _boundaryObj->getThermalVariableName(i));
        }
        layout->addWidget(toolBox);
        widget->setLayout(layout);
        _ui->tabWidget->addTab(widget, tr("Thermal"));
    }
}

