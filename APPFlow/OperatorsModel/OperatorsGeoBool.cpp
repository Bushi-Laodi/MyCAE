/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsGeoBool.h"

#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "GUIWidget/WidgetOCCEvent.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/GeometryBoolWidget.h"
#include "GUIDialog/GUIGeometryDialog/BoolInfoWidget.h"
#include "GUIDialog/GUIGeometryDialog/GeometryDeleteDialog.h"
#include "GUIDialog/GUIMeshDialog/MeshGeoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoOperBool.h"

namespace ModelOper {
    OperatorsGeoBool::OperatorsGeoBool()
    {

    }

    OperatorsGeoBool::~OperatorsGeoBool()
    {

    }

    bool OperatorsGeoBool::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return false;

        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;
        int objID = -1;
        this->argValue("objID", objID);

        GUI::BoolType type = GUI::BoolType::GeoBoolNone;
        if (_senderName == "actionGeoBoolFause") {
            type = GUI::BoolType::GeoBoolFause;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolCut") {
            type = GUI::BoolType::GeoBoolCut;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolCommon") {
            type = GUI::BoolType::GeoBoolCommon;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolOrImportEdit") {
            Interface::FITKAbsGeoCommand* obj = geometryData->getDataByID(objID);
            widget = new GUI::BoolInfoWidget(obj, this);
        }
        else if (_senderName == "actionGeoBoolOrImportDelete") {
            dialog = new GUI::GeometryDeleteDialog(dynamic_cast<Interface::FITKAbsGeoCommand*>(geometryData->getDataByID(objID)), this);
        }

        if (widget) {
            propertyWidget->setWidget(widget);
        }
        if (dialog) {
            dialog->show();
        }
        return false;
    }

    bool OperatorsGeoBool::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        if (_senderName == "actionGeoBoolOrImportDelete") {
            //更新网格划分区域界面
            GUI::MeshGeoWidget* widget = dynamic_cast<GUI::MeshGeoWidget*>(propertyWidget->getCurrentWidget());
            if (widget)widget->updateWidget();
        }
        _mainWindow->getPropertyWidget()->init();

        int objID, geo1ObjID, geo2ObjID = -1;
        this->argValue("geo1ObjID", geo1ObjID);
        this->argValue("geo2ObjID", geo2ObjID);
        this->argValue("objID", objID);

        graphOper->updateGraph(geo1ObjID);
        graphOper->updateGraph(geo2ObjID);
        graphOper->updateGraph(objID);
        treeOper->updateTree();
        graphOper->reRender(true);

        return true;
    }

    void OperatorsGeoBool::eventProcess(int index, QVariant value)
    {
        if (index == 0) {

        }
        //面组选择
        else if (index == 1) {
            int objID = -1;
            int curRow = -1;
            QList<int> faceIDs = {};
            this->argValue("objID", objID);
            this->argValue("curRow", curRow);
            this->argValue("faceIDs", faceIDs);

            //拾取对象获取事件绑定
            GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
            pickD->addDataManually(Interface::FITKModelEnum::FMSSurface, objID, faceIDs);
            if (pickD == nullptr) return;

            //拾取信息设置
            GUI::GUIPickInfoStru pinfo;
            pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJFace;
            pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMIndividually;
            //保存参数
            GUI::GUIPickInfo::SetPickInfo(pinfo, objID);

            EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
            if (graphOper == nullptr)return;
            graphOper->reRender();
        }
        //选择面组结束事件
        else if (index == 2) {
            slotSelectFaceGroup();
        }
    }

    void OperatorsGeoBool::slotSelectFaceGroup()
    {
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;

        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        GUI::GUIPickInfo::SetPickInfo(pinfo);

        QList<GraphData::PickedData*> pickData = pickD->getPickedList();

        QList<int> faceID = GUI::WidgetOCCEvent::getFaces(pickData);

        //界面获取
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;
        GUI::BoolInfoWidget* cudeWidget = dynamic_cast<GUI::BoolInfoWidget*>(propertyWidget->getCurrentWidget());
        if (cudeWidget == nullptr)return;

        int curRow = -1;
        this->argValue("curRow", curRow);
        cudeWidget->setFaceGroupValue(curRow, faceID);

        pickD->clearPickedData();

        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }
}

