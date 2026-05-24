/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef GUIWidgetBase_H
#define GUIWidgetBase_H

#include "GUIWidgetAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace GUI
{
    class MainWindow;

    class GUIWIDGETAPI GUIWidgetBase : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        GUIWidgetBase(QWidget* parent = nullptr);
        virtual ~GUIWidgetBase(); 

    protected:
        MainWindow* _mainWin = nullptr;
    };
}
#endif