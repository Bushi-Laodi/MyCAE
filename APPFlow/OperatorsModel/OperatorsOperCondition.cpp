/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsOperCondition.h"
#include "GUIFrame/PropertyWidget.h"

#include "GUIDialog/GUICalculateDialog/OperatingConditionWidget.h"

namespace ModelOper
{
    OperatorsOperCondition::OperatorsOperCondition()
    {

    }

    OperatorsOperCondition::~OperatorsOperCondition()
    {

    }

    bool OperatorsOperCondition::execGUI()
    {
        if (_mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = _mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::OperatingConditionWidget* widget = new GUI::OperatingConditionWidget(this, propertyWidget);
        propertyWidget->setWidget(widget);
        return true;
    }

    bool OperatorsOperCondition::execProfession()
    {
        return true;
    }
}
