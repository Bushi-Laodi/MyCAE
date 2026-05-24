/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _GUIDialogBase_H
#define _GUIDialogBase_H

#include "GUIWidgetAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace GUI
{
    class MainWindow;

    class GUIWIDGETAPI GUIDialogBase : public Core::FITKDialog
    {
        Q_OBJECT;
    public:
        GUIDialogBase(QWidget* parent = nullptr);
        virtual ~GUIDialogBase();

    protected:
        MainWindow* _mainWin = nullptr;
    };
}


#endif