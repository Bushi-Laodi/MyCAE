/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _RunProcess_H
#define _RunProcess_H

#include <QObject>
#include <QProcess>

namespace GUI
{
    class RunProcess :public QObject
    {
        Q_OBJECT;
    public:
        RunProcess();
        ~RunProcess();

        void start(QString sh);

        void kill();
    signals:
        ;
        void sigFinish();
    private slots:
        ;
        void slotProcessOutput();
        void slotProcessOutputError();
        void slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
        void slotMainwindowClose();
    private:
        QProcess* _process = nullptr;
    };
}
#endif
