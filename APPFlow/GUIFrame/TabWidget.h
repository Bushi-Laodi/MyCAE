/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _TabWidget_H
#define _TabWidget_H

#include "GUIFrameAPI.h"
#include <QTableWidget>

namespace GUI
{
    class GUIFRAMEAPI TabWidget :public QTabWidget
    {
        Q_OBJECT;
    public:
        TabWidget(QWidget* parent);
        ~TabWidget();
    };
}

#endif
