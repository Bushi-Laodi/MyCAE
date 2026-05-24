/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "PostWidget.h"
#include "ui_PostWidget.h"
#include "CalculateDriver.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKWorkBenchHandler.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Q_DECLARE_METATYPE(GUI::PostExportType)

namespace GUI
{
    PostWidget::PostWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _ui = new Ui::PostWidget();
        _ui->setupUi(this);

        init();
    }

    PostWidget::~PostWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void PostWidget::init()
    {
        //工作路径获取
        if (FITKAPP->getAppSettings()) {
            _workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (_workDir.isEmpty()) _workDir = QApplication::applicationDirPath() + "/../WorkDir";
        _caseDir = _workDir + "/case";
        Core::CreateDir(_caseDir);

        _ui->comboBox_Export->addItem(tr("VTK"), QVariant::fromValue(PostExportType::Post_VTK));
    }

    void GUI::PostWidget::on_pushButton_ParaView_clicked()
    {
        //创建paraView打开时间步文件
        QString foamFile = creatStartParaViewFile();
        if (foamFile.isEmpty())return;

        AppFrame::FITKProgramTaskManeger* proGramManager = FITKAPP->getProgramTaskManager();
        if (proGramManager == nullptr)return;
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "--case" << foamFile;
        info->setArgs(args);
        auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
        if (!progam) return;
        CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
        if (calDriver)calDriver->setExecProgram("paraview");
        //启动进程
        progam->start();
    }

    QString PostWidget::creatStartParaViewFile()
    {
        //创建case.foam文件用于paraView启动，查看后处理结果
        QString foamFile = _caseDir + "/case.foam";

        QFile file(foamFile);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 关闭文件
        file.close();
        return foamFile;
    }
}

void GUI::PostWidget::on_pushButton_Post_clicked()
{
    AppFrame::FITKProgramTaskManeger* proGramManager = FITKAPP->getProgramTaskManager();
    AppFrame::FITKAppSettings* appSetting = FITKAPP->getAppSettings();
    if (proGramManager == nullptr || appSetting == nullptr)return;
    QString casePath = appSetting->getWorkingDir() + "/case/VTK";
    QString postExePath = appSetting->getValue<QString>("CFDPostPath");
    if (postExePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("CFDPostPath is empty!"), QMessageBox::Ok);
        return;
    }
    AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
    QStringList args;
    auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
    if (!progam) return;
    CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
    if (calDriver == nullptr)return;
#ifdef Q_OS_WIN64 
    args << "-i" << casePath;
    info->setArgs(args);
    postExePath = postExePath + "/" + "CFDPostAPP.exe";
    calDriver->setExecProgram(postExePath);
#endif
#ifdef Q_OS_LINUX
    QString startCFDPostShFile = postExePath + "/startCFDPost.sh";
    QFile file(startCFDPostShFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "export MESA_GL_VERSION_OVERRIDE=4.5" << Qt::endl;
    out << "export MESA_GLSL_VERSION_OVERRIDE=450" << Qt::endl;
    out << QString("export LD_LIBRARY_PATH=%1:$LD_LIBRARY_PATH").arg(postExePath) << Qt::endl;
    out << QString("%1/CFDPostAPP -i %2").arg(postExePath).arg(casePath);
    file.close();
    
    args << startCFDPostShFile;
    info->setArgs(args);

    calDriver->setExecProgram("sh");
#endif
    //启动进程
    progam->start();
}

void GUI::PostWidget::on_pushButton_Export_clicked()
{
    auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
    auto proGramManager = app->getProgramTaskManager();
    AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
    QStringList args;
    args << "-ascii" << "-case" << _caseDir;
    info->setArgs(args);
    auto progam = proGramManager->createProgram(1, "CalculateDriver", info);
    if (!progam) return;
    CalculateDriver* calDriver = dynamic_cast<CalculateDriver*>(progam);
    if (calDriver) {
        PostExportType type = _ui->comboBox_Export->currentData().value<PostExportType>();
        switch (type) {
        case GUI::PostExportType::Post_VTK: {
            calDriver->setExecProgram("foamToVTK");
            break;
        }
        }
    }
    //启动进程
    progam->start();
    //写出到wb配置文件
    connect(progam, &AppFrame::FITKAbstractProgramerDriver::sig_Finish, this, [&] {
        if (!FITKAPP->workingInWorkBench()) return;
        AppFrame::FITKWorkBenchHandler* h = FITKAPP->getWorkBenchHandler();
        h->clearOutputInfo();
        AppFrame::IOFileInfo info;
        info._name = "case_";
        info._path = _caseDir+"/VTK";
        info._suffix = "vtk";
        info._describe = "OpenFOAM_Result";
        info._isFile = false;
        h->appendOutputInfo(info);
    });

}
