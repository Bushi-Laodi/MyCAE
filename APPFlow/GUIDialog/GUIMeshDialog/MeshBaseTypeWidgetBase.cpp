/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshBaseTypeWidgetBase.h"
#include "MeshBaseWidget.h"
#include "CompBaseBoundary.h"
#include "CompBaseBoundaryLineEdit.h"

#include "OperatorsInterface/GraphEventOperator.h"
#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"

namespace GUI
{
    MeshBaseTypeWidgetBase::MeshBaseTypeWidgetBase(QWidget* parent) :
        Core::FITKWidget(parent)
    {
        _meshBaseWidget = dynamic_cast<MeshBaseWidget*>(parent);
        _mainWin = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
    }

    MeshBaseTypeWidgetBase::~MeshBaseTypeWidgetBase()
    {

    }

    void MeshBaseTypeWidgetBase::slotMouseMove()
    {
        if (_graphObj == nullptr)return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        int rowIndex = -1;
        CompBaseBoundaryLabel* label = dynamic_cast<CompBaseBoundaryLabel*>(sender());
        CompBaseBoundaryComboBox* comBox = dynamic_cast<CompBaseBoundaryComboBox*>(sender());
        CompBaseBoundaryLineEdit* lineEdit = dynamic_cast<CompBaseBoundaryLineEdit*>(sender());
        QWidget* comParent = nullptr;
        if (label) {
            rowIndex = label->getPos();
            comParent = dynamic_cast<QWidget*>(label->parent());
        }
        else if (comBox) {
            rowIndex = comBox->getPos();
            comParent = dynamic_cast<QWidget*>(comBox->parent());
        }
        else if (lineEdit) {
            rowIndex = lineEdit->getPos();
            comParent = dynamic_cast<QWidget*>(lineEdit->parent());
        }

        if (comParent) {
            clearBoundaryBackgroudColor();
            comParent->setStyleSheet("background-color: #e0e0e0;");
        }

        //清除高亮
        graphOper->clearHighlight();

        getDataFromWidget(_graphObj);
        QVector<int> ids = {};
        ids.append(rowIndex);
        graphOper->advHighlight(_graphObj->getDataObjectID(), ids);
        graphOper->reRender(true);

    }

}
