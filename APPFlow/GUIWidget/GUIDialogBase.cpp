/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GUIDialogBase.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    GUI::GUIDialogBase::GUIDialogBase(QWidget* parent) :
        Core::FITKDialog(parent)
    {
        _mainWin = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
    }

    GUI::GUIDialogBase::~GUIDialogBase()
    {

    }
}

