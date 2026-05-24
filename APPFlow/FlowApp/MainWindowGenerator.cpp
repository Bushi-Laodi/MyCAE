/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MainWindowGenerator.h"
#include "GUIFrame/MainWindow.h"

MainWindowGenerator::MainWindowGenerator()
{
    //    this->setStyle("://Structural.qss");
    //    this->showMaximize(false);
}

QWidget * MainWindowGenerator::genMainWindow()
{
    return new GUI::MainWindow;
}
