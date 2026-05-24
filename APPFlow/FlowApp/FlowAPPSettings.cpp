/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "FlowAPPSettings.h"
#include <QSettings>
#include <QFile>

void FlowAPPSettings::read()
{
    //读入文件
    QString file = _appDir + "/FlowAPP.ini";
    QFile f(file);
    //文件不存在则创建
    if (!f.exists())
    {
        if (f.open(QFile::WriteOnly))
        {
            //创建文件
            f.write("");
            f.close();
        }
    }
    //读取文件
    _settings = new QSettings(file, QSettings::IniFormat);
}



