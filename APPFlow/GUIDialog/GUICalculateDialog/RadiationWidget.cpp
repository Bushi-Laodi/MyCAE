/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "RadiationWidget.h"
#include "ui_RadiationWidget.h"
#include "CalculateDriver.h"
#include "CompSelectComBoxWidget.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"

#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiation.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationParaManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationModel.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationAbsorptionModel.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationScatterModel.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationSunDirection.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRadiationSunLoad.h"

#include <QButtonGroup>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QTabBar>
#include <QLabel>
#include <QFrame>

namespace GUI
{
    Core::FITKParameter* radiationModelGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setRadiationModel(type);
        //获取辐射模型数据
        Interface::FITKOFRadiation* radiation = phyData->getRadiation();
        if (radiation == nullptr) return nullptr;
        Interface::FITKOFAbsRadiationModel * radiationModel = radiation->getRadiationModel();
        if (radiationModel == nullptr) return nullptr;

        return radiationModel->getRadiationModelPara();
    }

    Core::FITKParameter* radiationAbsorptionModelGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setRadiationAbsorptionModel(type);
        //获取辐射模型数据
        Interface::FITKOFRadiation* radiation = phyData->getRadiation();
        if (radiation == nullptr) return nullptr;
        Interface::FITKOFAbsRadiationAbsorptionModel * radiationAbsorptionModel = radiation->getAbsorptionModel();
        if (radiationAbsorptionModel == nullptr) return nullptr;

        return radiationAbsorptionModel->getRadiationAbsorptionModelPara();
    }

    Core::FITKParameter* radiationScatterModelGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setRadiationScatterModel(type);
        //获取辐射模型数据
        Interface::FITKOFRadiation* radiation = phyData->getRadiation();
        if (radiation == nullptr) return nullptr;
        Interface::FITKOFAbsRadiationScatterModel * radiationScatterModel = radiation->getScatterModel();
        if (radiationScatterModel == nullptr) return nullptr;

        return radiationScatterModel->getRadiationScatterModelPara();
    }

    Core::FITKParameter* radiationSunDirectionGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setRadiationScatterModel(type);
        //获取辐射模型数据
        Interface::FITKOFRadiation* radiation = phyData->getRadiation();
        if (radiation == nullptr) return nullptr;
        Interface::FITKOFAbsRadiationScatterModel * radiationScatterModel = radiation->getScatterModel();
        if (radiationScatterModel == nullptr) return nullptr;

        return radiationScatterModel->getRadiationScatterModelPara();
    }

    Core::FITKParameter* radiationSunLoadGetSubData(const QString & type, CompSelectComBoxWidget* widget)
    {
        if (widget == nullptr)return nullptr;
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;
        //工厂动作
        phyFactory->setRadiationScatterModel(type);
        //获取辐射模型数据
        Interface::FITKOFRadiation* radiation = phyData->getRadiation();
        if (radiation == nullptr) return nullptr;
        Interface::FITKOFAbsRadiationScatterModel * radiationScatterModel = radiation->getScatterModel();
        if (radiationScatterModel == nullptr) return nullptr;

        return radiationScatterModel->getRadiationScatterModelPara();
    }

    RadiationWidget::RadiationWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper,parent)
    {
        _ui = new Ui::RadiationWidget();
        _ui->setupUi(this);
        //获取数据
        if (_physicsData)
            _radiationObj = _physicsData->getRadiation();

        init();
    }

    RadiationWidget::~RadiationWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void RadiationWidget::init()
    {
        initRadiationPara();
        initModels();
        initSolarLoad();
    }

    void RadiationWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
    }

    void RadiationWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
    }

    void RadiationWidget::initRadiationPara()
    {
        if (!_radiationObj) return;
        Core::FITKParameter * radiationPara = _radiationObj->getRadiationParam();
        if (!radiationPara) return;
        //初始化辐射参数
        QList<Core::FITKAbstractEasyParam*> paramList = radiationPara->getParameter();
        for (auto value : paramList) {
            if (value == nullptr)continue;
            QWidget* widget = nullptr;
            switch (value->getParamType()) {
            case Core::FITKAbstractEasyParam::FEPBool:
            case Core::FITKAbstractEasyParam::FEPCombox:
            case Core::FITKAbstractEasyParam::FEPDouble:
            case Core::FITKAbstractEasyParam::FEPInt:
            case Core::FITKAbstractEasyParam::FEPLabel:
            case Core::FITKAbstractEasyParam::FEPString: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(value, this, value->getDataObjectName());
                break;
            }
            case Core::FITKAbstractEasyParam::FEPBoolGroup:
            case Core::FITKAbstractEasyParam::FEPComboxVector:
            case Core::FITKAbstractEasyParam::FEPDoubleList:
            case Core::FITKAbstractEasyParam::FEPRadioGroup: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(value, this);
                break;
            }
            }
            if (widget == nullptr) {
                continue;
            }
            _ui->verticalLayout_widget->addWidget(widget);
        }
    }

    void RadiationWidget::initModels()
    {
        if (_physicsManager == nullptr || !_radiationObj)return;
        Interface::FITKOFRadiationParaManager* radiationParaManager = _physicsManager->getRadiationParaManager();
        if (!radiationParaManager) return;
        //获取模型数据
        Interface::FITKOFAbsRadiationModel * radiationModel = _radiationObj->getRadiationModel();
        Interface::FITKOFAbsRadiationAbsorptionModel * absorptionModel = _radiationObj->getAbsorptionModel();
        Interface::FITKOFAbsRadiationScatterModel * scatterModel = _radiationObj->getScatterModel();
        if (!radiationModel && !absorptionModel && !scatterModel) return;
        QLabel* label = new QLabel;
        label->setText("Models");
        _ui->verticalLayout_widget->addWidget(label);
        QFrame* line = new QFrame(this);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _ui->verticalLayout_widget->addWidget(line);
        //辐射模型数据
        if (radiationModel)
        {
            QStringList options = radiationParaManager->filterRadiationModels(_physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Radiation Model", this);
            comp->setOptions(options);
            comp->setFunction(&radiationModelGetSubData);
            comp->setCurrentText(radiationModel->getDataObjectName());
            comp->setSubWidgetData(radiationModel->getRadiationModelPara());
            _ui->verticalLayout_widget->addWidget(comp);
        }
        //辐射吸收系数模型数据
        if (absorptionModel)
        {
            QStringList options = radiationParaManager->filterRadiationAbsorptionModels(_physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Absorption Model", this);
            comp->setOptions(options);
            comp->setFunction(&radiationAbsorptionModelGetSubData);
            comp->setCurrentText(absorptionModel->getDataObjectName());
            comp->setSubWidgetData(absorptionModel->getRadiationAbsorptionModelPara());
            _ui->verticalLayout_widget->addWidget(comp);
        }
        //辐射散射系数模型数据
        if (scatterModel)
        {
            QStringList options = radiationParaManager->filterRadiationScatterModels(_physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Scatter Model", this);
            comp->setOptions(options);
            comp->setFunction(&radiationScatterModelGetSubData);
            comp->setCurrentText(scatterModel->getDataObjectName());
            comp->setSubWidgetData(scatterModel->getRadiationScatterModelPara());
            _ui->verticalLayout_widget->addWidget(comp);
        }
    }

    void RadiationWidget::initSolarLoad()
    {
        if (_physicsManager == nullptr || !_radiationObj)return;
        Interface::FITKOFRadiationParaManager* radiationParaManager = _physicsManager->getRadiationParaManager();
        if (!radiationParaManager) return;
        //获取太阳源模型数据
        Interface::FITKOFRadiationSolarLoad * solarLoad = _radiationObj->getSolarLoad();
        if (!solarLoad) return;
        //Solar Load
        QLabel* label = new QLabel;
        label->setText("Solar Load");
        _ui->verticalLayout_widget->addWidget(label);
        QFrame* line = new QFrame(this);
        line->setObjectName(QString::fromUtf8("line_2"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _ui->verticalLayout_widget->addWidget(line);
        //初始化太阳源辐射参数
        Core::FITKParameter * solarLoadParam = solarLoad->getRadiationSolarLoadParams();
        if (!solarLoad) return;
        QList<Core::FITKAbstractEasyParam*> paramList = solarLoadParam->getParameter();
        for (auto value : paramList) {
            if (value == nullptr)continue;
            QWidget* widget = nullptr;
            switch (value->getParamType()) {
            case Core::FITKAbstractEasyParam::FEPBool:
            case Core::FITKAbstractEasyParam::FEPCombox:
            case Core::FITKAbstractEasyParam::FEPDouble:
            case Core::FITKAbstractEasyParam::FEPInt:
            case Core::FITKAbstractEasyParam::FEPLabel:
            case Core::FITKAbstractEasyParam::FEPString: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(value, this, value->getDataObjectName());
                break;
            }
            case Core::FITKAbstractEasyParam::FEPBoolGroup:
            case Core::FITKAbstractEasyParam::FEPComboxVector:
            case Core::FITKAbstractEasyParam::FEPDoubleList:
            case Core::FITKAbstractEasyParam::FEPRadioGroup: {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(value, this);
                break;
            }
            }
            if (widget == nullptr) {
                continue;
            }
            _ui->verticalLayout_widget->addWidget(widget);
        }
        //太阳源方向
        Interface::FITKOFAbsRadiationSunDirection * sunDire = solarLoad->getRadiationSunDirection();
        if (sunDire)
        {
            QStringList options = radiationParaManager->filterRadiationSunDirections(_physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Sun Direction", this);
            comp->setOptions(options);
            comp->setFunction(&radiationScatterModelGetSubData);
            comp->setCurrentText(sunDire->getDataObjectName());
            comp->setSubWidgetData(sunDire->getRadiationSunDirectionPara());
            _ui->verticalLayout_widget->addWidget(comp);
        }
        //太阳源载荷
        Interface::FITKOFAbsRadiationSunLoad * sunLoad = solarLoad->getRadiationSunLoad();
        if (sunLoad)
        {
            QStringList options = radiationParaManager->filterRadiationSunLoads(_physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget("Sun Load", this);
            comp->setOptions(options);
            comp->setFunction(&radiationScatterModelGetSubData);
            comp->setCurrentText(sunLoad->getDataObjectName());
            comp->setSubWidgetData(sunLoad->getRadiationSunLoadPara());
            _ui->verticalLayout_widget->addWidget(comp);
        }
    }

}
