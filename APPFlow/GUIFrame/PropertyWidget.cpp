/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "PropertyWidget.h"
#include <QVariant>
#include "MainWindow.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDebug>
#include "GUIWidget/PropertyEmptyWidget.h"

namespace GUI
{
    PropertyWidget::PropertyWidget(MainWindow *mainWindow) : PanelWidgetBase(mainWindow)
    {
        this->setTitle(tr("Property Area"));
        init();
    }

    void PropertyWidget::init()
    {
        //auto scrollArea = new QScrollArea(this);
        //scrollArea->setWidget();
        setWidget(new PropertyEmptyWidget(m_MainWindow, tr("Welcome to FlowApp.")));
    }
}
