/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "SolutionWidget.h"
#include "ui_SolutionWidget.h"
#include "CompSelectComBoxWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetVBox.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolution.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolutionSolverManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolutionSolver.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"

#include <QToolBox>

namespace GUI
{
    Core::FITKParameter* solutionGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        int index = widget->getData("index").toInt();
        phyFactory->setSolutionSolver(index, type);
        Interface::FITKOFAlgebraicEquationsPara* solution = phyData->getSolution()->getSolverVariablePara(index);
        if (solution == nullptr)return nullptr;
        Interface::FITKOFAbsSolutionSolver* solver = solution->getSolutionSolver();
        if (solver == nullptr)return nullptr;

        return solver->getSolverSolutionPara();
    }

    SolutionWidget::SolutionWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::SolutionWidget();
        _ui->setupUi(this);

        init();
    }

    SolutionWidget::~SolutionWidget()
    {
        if (_ui) delete _ui;
    }

    void SolutionWidget::init()
    {
        if (_physicsData == nullptr)return;
        _solValue = _physicsData->getSolution();

        updateSlovers();
        updateSlover();
        updateResiduals();
        updateRelaxation();
        updateLimits();
    }

    void SolutionWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void SolutionWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void SolutionWidget::updateSlovers()
    {
        if (_physicsManager == nullptr)return;
        auto solutionManager = _physicsManager->getSolutionSolverManager();
        if (solutionManager == nullptr)return;
        if (_factoryData == nullptr)return;
        if (_solValue == nullptr)return;
        int solversNum = _solValue->getSolversCount();

        QTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        for (int i = 0; i < solversNum; i++) {
            Interface::FITKOFAlgebraicEquationsPara* solversData = _solValue->getSolverVariablePara(i);
            if(solversData == nullptr)continue;
            QString type = solversData->getVariableName();
            QList<QWidget*> widgetList = {};
            //其余数据添加
            Core::FITKParameter* solverOther = solversData->getSolverAdditionalPara();
            if(solverOther){
                for (auto d : solverOther->getParameter()) {
                    if (d == nullptr)continue;
                    QWidget* w = new Core::FITKWidgetComLine(d, this);
                    widgetList.append(w);
                }
            }
            //Solver数据添加
            Interface::FITKOFAbsSolutionSolver* solver = solversData->getSolutionSolver();
            if (solver) {
                QStringList options = solutionManager->filterSolutionSolvers(type, _physicsData->getSolver()->getSolverType());
                CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Solver", tabWidget);
                comp->setData("index", i);
                comp->setFunction(&solutionGetSubData);
                comp->setOptions(options);
                comp->setCurrentText(solver->getDataObjectName());
                comp->setSubWidgetData(solver->getSolverSolutionPara());
                widgetList.append(comp);
            }
            Core::FITKWidgetVBox* widget = new Core::FITKWidgetVBox(widgetList, this);
            if (widget == nullptr) {
                continue;
            }
            //添加弹簧
            QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            widget->layout()->addItem(spacer);
            tabWidget->addTab(widget, type);
        }
        //添加界面
        _ui->verticalLayout_Solvers->addWidget(tabWidget);
    }

    void SolutionWidget::updateSlover()
    {
        if (_physicsData == nullptr)return;
        auto solver = _physicsData->getSolver();
        if (solver == nullptr)return;
        _ui->tabWidget->setTabText(1, _solValue->getSolverSpeciallyDataName());
        if (_solValue == nullptr)return;

        auto solverValue = _solValue->getSolverSpeciallyDataPara();
        if (solverValue == nullptr)return;
        
        for (auto value : solverValue->getParameter()) {
            if (value == nullptr)continue;
            QWidget* widget = new Core::FITKWidgetComLine(value, this);
            _ui->verticalLayout_Solver->addWidget(widget);
        }
    }

    void SolutionWidget::updateResiduals()
    {
        if (_solValue == nullptr)return;

        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        int resNum = _solValue->getResidualsCount();
        for (int i = 0; i < resNum; i++) {
            QString name = _solValue->getResidualVariableName(i);
            auto value = _solValue->getResidualVariablePara(i);
            QList<QWidget*> widgets = {};
            for (auto v : value->getParameter()) {
                if (v == nullptr)return;
                QWidget* widget = new Core::FITKWidgetComLine(v, this);
                widgets.append(widget);
            }
            Core::FITKWidgetVBox* VBoxWidget = new Core::FITKWidgetVBox(widgets, this);
            if (VBoxWidget == nullptr) {
                continue;
            }
            //添加弹簧
            QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            VBoxWidget->layout()->addItem(spacer);
            tabWidget->addTab(VBoxWidget, name);
        }
        //添加界面
        _ui->verticalLayout_Residuals->addWidget(tabWidget);
    }

    void SolutionWidget::updateRelaxation()
    {
        if (_solValue == nullptr)return;
        auto relValue = _solValue->getRelaxation();
        if (relValue == nullptr)return;

        for (auto v : relValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = new Core::FITKWidgetComLine(v, this);
            _ui->verticalLayout_Relaxation->addWidget(widget);
        }
    }

    void SolutionWidget::updateLimits()
    {
        if (_solValue == nullptr)return;
        auto limValue = _solValue->getLimits();
        if (limValue == nullptr)return;

        for (Core::FITKAbstractEasyParam* v : limValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = nullptr;
            switch (v->getParamType()){
            case Core::FITKAbstractEasyParam::FEPBool:
            case Core::FITKAbstractEasyParam::FEPCombox:
            case Core::FITKAbstractEasyParam::FEPDouble:
            case Core::FITKAbstractEasyParam::FEPInt:
            case Core::FITKAbstractEasyParam::FEPLabel:
            case Core::FITKAbstractEasyParam::FEPString: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(v, this, v->getDataObjectName());
                break;
            }
            case Core::FITKAbstractEasyParam::FEPBoolGroup:
            case Core::FITKAbstractEasyParam::FEPComboxVector:
            case Core::FITKAbstractEasyParam::FEPDoubleList:
            case Core::FITKAbstractEasyParam::FEPRadioGroup: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(v, this);
                break;
            }
            }
            if (widget == nullptr) {
                continue;
            }
            _ui->verticalLayout_Limits->addWidget(widget);
        }
    }
}

