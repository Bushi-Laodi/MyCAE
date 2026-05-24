/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GroupPropertyWidget.h"
#include "FITK_Component/FITKWidget/FITKDrawerWidget.h"

namespace GUI
{
    GroupPropertyWidget::GroupPropertyWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
    {
        this->setTitle(tr("Objects Manage"));
        auto drawerWidget = new Comp::FITKDrawerWidget(this);
        drawerWidget->setAllItemFold(true); ///所有抽屉可同时关闭
        drawerWidget->setLockerWidgetIcon(QPixmap(":/icons/fold.png"), QPixmap(":/icons/gfold.png"));
        drawerWidget->addWidget(new QWidget(nullptr), "test 1");
        drawerWidget->addWidget(new QWidget, "test 2");
        drawerWidget->addWidget(new QWidget, "test 3");
        drawerWidget->addWidget(new QWidget, "test 4");
        drawerWidget->addWidget(new QWidget, "test 5");

        this->setWidget(drawerWidget);
    }

    GroupPropertyWidget::~GroupPropertyWidget()
    {

    }
}
