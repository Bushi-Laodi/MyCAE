/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "TabWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKPython/FITKPythonInterface.h"
#include "FITK_Component/FITKCompMessageWidget/FITKConsoleComponent.h"

namespace GUI
{
    TabWidget::TabWidget(QWidget * parent)
    {
        // 尝试从FITK应用程序中获取名为"MessageConsole"的组件，并将其添加到UI的tab_message标签页中
        auto conWidgetComp = FITKAPP->getComponents()->getComponentByName("MessageConsole");
        Comp::ConsoleComponent *conWidgetInter = dynamic_cast<Comp::ConsoleComponent *>(conWidgetComp);
        if (nullptr != conWidgetInter) {
            this->addTab(conWidgetInter->getWidget(0), tr("Information"));
        }

        // 获取Python界面实例，并将其添加到UI的tab_python标签页中
        auto pyInter = Python::FITKPythonInterface::getInstance();
        this->addTab(pyInter->getPyWidget(), "Python");
    }

    TabWidget::~TabWidget()
    {

    }
}

