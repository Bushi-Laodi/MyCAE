/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "ControlPanelWidget.h"
#include "MainWindow.h"
#include "FITK_Component/FITKWidget/FITKDrawerWidget.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QPixmap>

namespace GUI
{
    ControlPanelWidget::ControlPanelWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
    {
        this->setTitle(tr("Control Panel"));
        auto drawerWidget = new Comp::FITKDrawerWidget(this);

        drawerWidget->setLockerWidgetIcon(QPixmap(":/icons/fold.png"), QPixmap(":/icons/gfold.png"));
        drawerWidget->setAllItemFold(true); ///所有抽屉可同时关闭

        drawerWidget->addWidget(new QWidget(nullptr), "test 1");

        drawerWidget->addWidget(new QWidget, "test 2");
        drawerWidget->addWidget(new QWidget, "test 3");
        drawerWidget->addWidget(new QWidget, "test 4");
        drawerWidget->addWidget(new QWidget, "test 5");

        this->setWidget(drawerWidget);
    }

    ControlPanelWidget::~ControlPanelWidget()
    {

    }

}
