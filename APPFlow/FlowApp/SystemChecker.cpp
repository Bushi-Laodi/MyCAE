/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "SystemChecker.h"
#include <QThread>

QStringList SystemChecker::check()
{
    QStringList s;
    //线程数检查，最小是4
    const int nThread = QThread::idealThreadCount();
    if (nThread < 4)
        s << QString("CPU thread count is %1, at least 4 !").arg(nThread);
    return s;
}

