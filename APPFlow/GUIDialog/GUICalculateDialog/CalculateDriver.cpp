/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CalculateDriver.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include <QProcess>

namespace GUI
{
    CalculateDriver::CalculateDriver()
    {
        _process = new QProcess(this);
        MainWindow* mianWindow = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        connect(_process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
        connect(_process, SIGNAL(readyReadStandardError()), this, SLOT(slotProcessOutputError()));
        connect(_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProcessError(QProcess::ProcessError)));
        connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinish(int, QProcess::ExitStatus)));
        connect(mianWindow, SIGNAL(sigMainWindowClose()), this, SLOT(slotMainwindowClose()));
    }

    CalculateDriver::~CalculateDriver()
    {
        if (_process) {
            _process->kill();
            _process->waitForFinished();
            delete _process;
            _process = nullptr;
        }
    }

    void CalculateDriver::setExecProgram(const QString & program)
    {
        _program = program;
    }

    QString CalculateDriver::getExecProgram() const
    {
        return _program;
    }

    int CalculateDriver::getProgramType()
    {
        return 0;
    }

    QString CalculateDriver::getProgramName()
    {
        return QString();
    }

    void CalculateDriver::start()
    {
        //启动进程
        if (_process == nullptr) return;
        QStringList args;
        if (_inputInfo != nullptr)
            args = _inputInfo->args();

        //信号关联
        _process->start(_program, args);
        //判断进程是否启动成功
        if (!_process->waitForStarted()) {
            QString message = tr("Started failed!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
            emit sig_Finish();
        }
        else {
            QString message = tr("Started successfully!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(1, message);
        }
    }

    void CalculateDriver::stop()
    {
        //终止进程
        if (_process == nullptr) return;
        _process->kill();
        _process->waitForFinished();
    }

    void CalculateDriver::slotProcessOutput()
    {
        QString message = _process->readAllStandardOutput();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(4, message);
    }

    void CalculateDriver::slotProcessOutputError()
    {
        QString message = _process->readAllStandardError();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
    }

    void CalculateDriver::slotProcessError(QProcess::ProcessError error)
    {
        QString message = _process->readAllStandardError();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
    }

    void CalculateDriver::slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
    {
        switch (exitStatus)
        {
        case QProcess::NormalExit:
            break;
        case QProcess::CrashExit:
            break;
        }
        emit sig_Finish();
    }

    void CalculateDriver::slotMainwindowClose()
    {
        this->stop();
    }
}

