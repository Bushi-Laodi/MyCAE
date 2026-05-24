/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "RunWidget.h"
#include "ui_RunWidget.h"
#include "CalculateDriver.h"

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
#include "FITK_Component/FITKOFDictWriter/FITKOFDictWriterIO.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRunControl.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredMeshVTK.h"

#include <QButtonGroup>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QTabBar>

#define CPUType "CPUType"
Q_DECLARE_METATYPE(GUI::RunCPUType)

namespace GUI
{
    static RunCPUType _currentCUPType = RunCPUType::Serial;
    static int _currentCUPNum = 4;
    static int _currentDriverID = -1;

    RunWidget::RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper,parent)
    {
        _ui = new Ui::RunWidget();
        _ui->setupUi(this);

        init();
    }

    RunWidget::~RunWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void RunWidget::init()
    {
        if (_physicsData) {
            _runConObj = _physicsData->getRunControl();
        }
        updataTime();
        updateOutput();
        initCPU();
    }

    void RunWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void RunWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void RunWidget::slotCPUChange(QAbstractButton * button)
    {
        if (button == nullptr)return;
        _currentCUPType = button->property(CPUType).value<RunCPUType>();
        updateCPU();
    }

    void RunWidget::slotProcessFinish()
    {
        setRunType(false);
    }

    void RunWidget::on_spinBox_NumOfPro_valueChanged(int arg1)
    {
        _currentCUPNum = arg1;
        updateCPU();
    }

    void RunWidget::on_pushButton_Stop_clicked()
    {
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = app->getProgramTaskManager();
        auto driver = proGramManager->getDataByID(_currentDriverID);
        if (driver) {
            driver->stop();
        }
        setRunType(false);
    }

    void RunWidget::on_pushButton_Run_clicked()
    {
        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";
        QString caseDir = workDir + "/case";

        //清理字典文件
        if (!clearCasePath(caseDir)) {
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, tr("Clear failed!"));
            return;
        }
        //写出字典文件
        if (!writeCase(caseDir)) {
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, tr("Write failed!"));
            return;
        }
        //写出启动脚本
        QString shPath = creatStartSh(workDir, caseDir);
        if (shPath.isEmpty()) {
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, tr("Create sh failed!"));
            return;
        }

        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = app->getProgramTaskManager();
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << shPath;
        info->setArgs(args);
        auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
        if (!progam) return;
        CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
        if (calDriver) {
            calDriver->setExecProgram("/bin/bash");
            _currentDriverID = calDriver->getDataObjectID();
        }
        //启动进程
        connect(progam, SIGNAL(sig_Finish()), this, SLOT(slotProcessFinish()));
        connect(progam, &CalculateDriver::sig_Finish, []() {
            auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
            auto proGramManager = app->getProgramTaskManager();
            if (proGramManager)proGramManager->removeDataByID(_currentDriverID);
        });

        setRunType(true);
        progam->start();
    }

    void RunWidget::updataTime()
    {
        if (_runConObj == nullptr)return;
        Core::FITKParameter* tiemPara = _runConObj->getTimeControl();
        for (auto data : tiemPara->getParameter()) {
            if(data == nullptr)continue;
            QWidget* widget = nullptr;
            if (data->getParamType() == Core::FITKAbstractEasyParam::FITKEasyParamType::FEPBoolGroup) {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(data, this);
            }
            else {
                widget = new Core::FITKWidgetComLine(data, this);
            }
            _ui->verticalLayout_Time->addWidget(widget);
        }
    }

    void RunWidget::updateOutput()
    {
        if (_runConObj == nullptr)return;
        Core::FITKParameter* outPara = _runConObj->getOutputControl();
        for (auto data : outPara->getParameter()) {
            if (data == nullptr)continue;
            QWidget* widget = nullptr;
            if (data->getParamType() == Core::FITKAbstractEasyParam::FITKEasyParamType::FEPBoolGroup) {
                widget = Core::FITKEasyParamWidgetFactory::createWidget(data, this);
            }
            else {
                widget = new Core::FITKWidgetComLine(data, this);
            }
            _ui->verticalLayout_Output->addWidget(widget);
        }
    }

    void RunWidget::initCPU()
    {
        QButtonGroup* group = new QButtonGroup(this);
        _ui->radioButton_serial->setProperty(CPUType, QVariant::fromValue(RunCPUType::Serial));
        _ui->radioButton_parallel->setProperty(CPUType, QVariant::fromValue(RunCPUType::Parallel));
        group->addButton(_ui->radioButton_serial);
        group->addButton(_ui->radioButton_parallel);
        connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotCPUChange(QAbstractButton*)));
        updateCPU();

        //线程事件未结束，重新绑定结束事件与设置界面
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        auto proGramManager = app->getProgramTaskManager();
        auto progam = proGramManager->getDataByID(_currentDriverID);
        if (progam) {
            //启动进程
            connect(progam, SIGNAL(sig_Finish()), this, SLOT(slotProcessFinish()));
        }
        setRunType(progam);
    }

    void RunWidget::updateCPU()
    {
        switch (_currentCUPType) {
        case GUI::RunCPUType::Serial: {
            _ui->spinBox_NumOfPro->setEnabled(false);
            _ui->radioButton_serial->setChecked(true);
            break;
        }
        case GUI::RunCPUType::Parallel: {
            _ui->spinBox_NumOfPro->setEnabled(true);
            _ui->radioButton_parallel->setChecked(true);
            break;
        }
        }
        _ui->spinBox_NumOfPro->setValue(_currentCUPNum);
    }

    bool RunWidget::clearCasePath(QString casePath)
    {
        QDir dir(casePath);
        // 设置过滤器，只获取目录
        dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        // 获取目录列表
        QStringList directories = dir.entryList();

        //清除非system与constant文件夹
        for (auto path : directories) {
            if(path == "system")continue;
            if(path == "constant")continue;
            Core::RemoveDir(casePath + "/" + path);
        }

        return true;
    }

    bool RunWidget::writeCase(QString casePath)
    {
        auto dicWriComp = FITKAPP->getComponents()->getComponentTByName<IO::FITKOFDictWriterIO>("IO::FITKOFDictWriterIO");
        if (dicWriComp == nullptr)return false;

        if (!dicWriComp->setFilePath(casePath)) {
            Core::CreateDir(casePath);
            dicWriComp->setFilePath(casePath);
        }
        dicWriComp->setPhysicsDictW();
        if (!dicWriComp->exec())return false;

        return true;
    }

    QString RunWidget::creatStartSh(QString workDir, QString caseDir)
    {
        QString foamRun, foamName;
        if (_factoryData == nullptr)return "";
        bool isSetFields = _factoryData->isExecuteSetFields();
        Interface::FITKAbstractOFSolver* curSolver = _physicsData->getSolver();
        if (curSolver) {
            foamName = curSolver->getSolverCommand();
        }
        
        //脚本生成
        switch (_currentCUPType) {
        case GUI::RunCPUType::Serial:break;
        case GUI::RunCPUType::Parallel: {
            foamRun += QString("mpirun -np %1 ").arg(_currentCUPNum);
            break;
        }
        }
        foamRun += QString("%1 -case %2").arg(foamName).arg(caseDir);

        QString shFilePath = workDir + "/startOpenFoam.sh";
        QFile file(shFilePath);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 写入字符串到文件
        if (isSetFields) {
            out << QString("setFields -case %1").arg(caseDir);
            out << QStringLiteral("\n");
        }

        //写出特定区域处理
        QStringList regionScriptCommand = this->getRegionTouch(caseDir);
        for (QString strCommand : regionScriptCommand)
            out << strCommand << endl;
        out << foamRun << endl;
        // 关闭文件
        file.close();
        return shFilePath;
    }

    void RunWidget::setRunType(bool isRun)
    {
        if (isRun) {
            _ui->progressBar->show();
            _ui->pushButton_Run->setEnabled(false);
            _ui->pushButton_Stop->setEnabled(true);
        }
        else
        {
            _ui->progressBar->hide();
            _ui->pushButton_Run->setEnabled(true);
            _ui->pushButton_Stop->setEnabled(false);
        }
    }

    QStringList RunWidget::getRegionTouch(QString caseDir)
    {
        Interface::FITKOFPhysicsData* physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        Interface::FITKUnstructuredFluidMeshVTK* meshData = FITKAPP->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        if (!physicsData || !meshData) return QStringList();
        Interface::FITKAbstractOFSolver* solver = physicsData->getSolver();
        if (!solver) return QStringList();
        Interface::FITKOFSolverTypeEnum::FITKOFSolverType solverType = solver->getSolverType();
        QStringList script;
        if (solverType == Interface::FITKOFSolverTypeEnum::FITKOFSolverType::CHT_MULTI_REGION)
        {
            int count = meshData->getDataCount();
            for (int i = 0; i < count; ++i)
            {
                auto regionMesh = meshData->getDataByIndex(i);
                if (!regionMesh) continue;
                if (physicsData->getRegionMeshType(regionMesh->getDataObjectID()) == Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::None) continue;
                QString name = regionMesh->getDataObjectName();
                script << QString("paraFoam -case %1 -region %2 -touch").arg(caseDir).arg(name);
            }
        }

        return script;
    }
}
