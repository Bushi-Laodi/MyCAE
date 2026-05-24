/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompFaceGroupWidget.h"
#include "ui_CompFaceGroupWidget.h"
#include "CompFaceGroupSelectWidget.h"
#include "GeometryWidgetBase.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoShapeAgent.h"
#include "FITK_Interface/FITKInterfaceModel/FITKAbstractModel.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpacerItem>

#define GeoComObjID Qt::UserRole

namespace GUI 
{
    CompFaceGroupWidget::CompFaceGroupWidget(QWidget* paraent, Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper) :
        GUIWidgetBase(paraent), _obj(obj), _oper(oper)
    {
        _parentWidget = dynamic_cast<GeometryWidgetBase*>(paraent);
        init();
    }

    CompFaceGroupWidget::~CompFaceGroupWidget()
    {
        clearTableWidget();
        if (_ui)delete _ui;
    }

    void CompFaceGroupWidget::init()
    {
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;

        _ui = new Ui::CompFaceGroupWidget();
        _ui->setupUi(this);

        initTableWidget();
        setDataToWidget();
    }

    void CompFaceGroupWidget::setFaceGroupValue(int rowIndex, QList<int> facesId, bool r )
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupSelectWidget* item = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(rowIndex, 0));
        if (item == nullptr)return;
        auto obj = commanger->getDataByID(item->data(GeoComObjID).toInt());
        if (obj == nullptr)return;

        //重新设置面id
        QString name = obj->getDataObjectName();
        obj->setMember(facesId);
        if (facesId.size() == 0) {
            name += tr("(empty)");
        }
        else {
            name += tr("(%1 faces)").arg(facesId.size());
        }
        item->setName(name);

        if (!r) return;
        //处理其他面组
        for (int faceId : facesId) {
            for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
                if (i == rowIndex)continue;
                CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
                if (otherItem == nullptr)continue;
                auto otherObj = commanger->getDataByID(otherItem->data(GeoComObjID).toInt());
                if (otherObj == nullptr)continue;
                QList<int> ids = otherObj->getMember();

                //如果已经包含该id,移除该id，并重新设置名称
                if (ids.contains(faceId)) {
                    ids.removeOne(faceId);

                    QString otherName = otherObj->getDataObjectName();
                    otherObj->setMember(ids);
                    if (ids.size() == 0) {
                        otherName += tr("(empty)");
                    }
                    else {
                        otherName += tr("(%1 faces)").arg(ids.size());
                    }
                    otherItem->setName(otherName);
                }
            }
        }

        //更新default面组id
        updateDefaultFaceGroupIDs();
    }

    Interface::FITKAbsGeoCommand * CompFaceGroupWidget::getCurrentGeoCommand()
    {
        return _obj;
    }

    void CompFaceGroupWidget::on_pushButton_Clear_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            //Default面组不可编辑
            if (i == 0)continue;
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            commanger->removeDataByID(widget->data(GeoComObjID).toInt());
        }

        clearTableWidget();
        //Default面组刷新
        updateDefaultFaceGroupIDs();
        clearGraphHight();
    }

    void CompFaceGroupWidget::on_pushButton_Add_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString group = commanger->checkName(tr("Group-1"));
        QString name = group + tr("(empty)");

        //创建面组对象
        Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
        geoCom->setDataObjectName(group);
        commanger->appendDataObj(geoCom);

        CompFaceGroupSelectWidget* widget = new CompFaceGroupSelectWidget(_ui->tableWidget);
        widget->setName(name);
        widget->setData(GeoComObjID, geoCom->getDataObjectID());
        _ui->tableWidget->setCellWidget(rowNum, 0, widget);

        connect(widget, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
        connect(widget, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
        connect(widget, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
        connect(widget, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
        connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));

        //更新位置
        updateFaceWidgetCurrentPos();
    }

    void CompFaceGroupWidget::slotCellTableClicked(int row, int column)
    {
        //Default面组不可编辑
        if (row == 0) {
            clearGraphHight();
            //拾取信息设置
            GUI::GUIPickInfoStru pinfo;
            pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
            pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
            GUI::GUIPickInfo::SetPickInfo(pinfo);
            //移除defaultIDs中其他面组的id
            for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
                CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
                if (otherItem == nullptr)continue;
                otherItem->setSelect(false);
            }
            return;
        }

        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;

        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(row, column));
        if (widget == nullptr)return;
        auto obj = commanger->getDataByID(widget->data(GeoComObjID).toInt());
        if (obj == nullptr)return;

        if (_oper) {
            setAllFaceGroupSelect(false);
            widget->setSelect(true);
            clearGraphHight();
            //执行操作器
            _oper->setArgs("objID", _obj->getDataObjectID());
            _oper->setArgs("faceIDs", QVariant::fromValue(obj->getMember()));
            _oper->eventProcess(1);
        }
    }

    void CompFaceGroupWidget::slotEditNameStart()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;

        auto obj = commanger->getDataByID(widget->data(GeoComObjID).toInt());
        if (obj == nullptr)return;
        widget->setName(obj->getDataObjectName());
    }

    void CompFaceGroupWidget::slotEditNameFinish()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;
        auto obj = commanger->getDataByID(widget->data(GeoComObjID).toInt());
        if (obj == nullptr)return;
        int objID = obj->getDataObjectID();
        QString name = "";
        //判断新名称是否存在
        if (commanger->getDataByName(widget->getName())) {
            name = obj->getDataObjectName();
        }
        else {
            name = widget->getName();
        }

        //设置控件名称
        obj->setDataObjectName(name);
        if (obj->getMember().size() == 0) {
            name += tr("(empty)");
        }
        else {
            name += tr("(%1 faces)").arg(obj->getMember().size());
        }
        widget->setName(name);

        //更新网格边界对象对应的名称
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        auto meshSizeManger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        for (int i = 0; i < meshSizeManger->getDataCount(); i++) {
            auto meshSizeObj = meshSizeManger->getDataByIndex(i);
            if (meshSizeObj == nullptr)continue;
            if (meshSizeObj->getGeoGroupComponentId() == objID) {
                QString meshSizeNewName = _obj->getDataObjectName() + "." + obj->getDataObjectName();
                meshSizeObj->setDataObjectName(meshSizeNewName);
                treeOper->updateTree();
                break;
            }
        }
    }

    void CompFaceGroupWidget::slotFaceWidgetOkClicked()
    {
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;
        //执行选择结束事件
        if (_oper) {
            _oper->setArgs("curRow", widget->getCurrentPos().first);
            _oper->eventProcess(2);
            widget->setSelect(false);
        }

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    void CompFaceGroupWidget::slotFaceWidgetCancelClicked()
    {
        int currentRow = _ui->tableWidget->currentRow();
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(currentRow, 0));
        if (widget == nullptr) return;

        widget->setSelect(false);
        //清除高亮
        clearGraphHight();

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    void CompFaceGroupWidget::slotFaceWidgetDeleteClicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(sender());
        if (widget == nullptr) return;

        int objID = widget->data(GeoComObjID).toInt();
        commanger->removeDataByID(objID);
        _ui->tableWidget->removeRow(widget->getCurrentPos().first);

        //更新界面中存储的位置
        updateFaceWidgetCurrentPos();
        //更新default面组id
        updateDefaultFaceGroupIDs();
        //清除高亮
        clearGraphHight();

        //判断当前面组是否被网格边界参数所使用，被使用移除对应的网格边界参数对象
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        auto meshSizeManger = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        for (int i = 0; i < meshSizeManger->getDataCount(); i++) {
            auto meshSizeObj = meshSizeManger->getDataByIndex(i);
            if (meshSizeObj == nullptr)continue;
            if (meshSizeObj->getGeoGroupComponentId() == objID) {
                meshSizeManger->removeDataByID(meshSizeObj->getDataObjectID());
                treeOper->updateTree();
                break;
            }
        }

        _ui->tableWidget->setCurrentCell(-1, -1);
    }

    bool CompFaceGroupWidget::checkValue()
    {
        auto outputMessage = [&](QString name, double value) {
            QMessageBox::critical(this, "", tr("%1 value : %2 error!").arg(name).arg(value), QMessageBox::Ok);
        };
        return true;
    }

    void CompFaceGroupWidget::setDataToWidget()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        if (commanger == nullptr)return;
        _ui->tableWidget->setRowCount(commanger->getDataCount());
        for (int i = 0; i < commanger->getDataCount(); i++) {
            Interface::FITKGeoComponent* geoCom = dynamic_cast<Interface::FITKGeoComponent*>(commanger->getDataByIndex(i));
            if (geoCom == nullptr)continue;
            QList<int> ids = geoCom->getMember();
            QString name = geoCom->getDataObjectName();

            CompFaceGroupSelectWidget* item = new CompFaceGroupSelectWidget(_ui->tableWidget);
            _ui->tableWidget->setCellWidget(i, 0, item);
            if (ids.size() == 0) {
                name += tr("(empty)");
            }
            else {
                name += tr("(%1 faces)").arg(ids.size());
            }
            item->setName(name);
            item->setData(GeoComObjID, geoCom->getDataObjectID());
            item->setCurrentPos(i, 0);

            if (i == 0) {
                //Default面组不可编辑
                item->setIsEdit(false);
            }
            else
            {
                connect(item, SIGNAL(sigEditNameStart()), this, SLOT(slotEditNameStart()));
                connect(item, SIGNAL(sigEditNameFinish()), this, SLOT(slotEditNameFinish()));
                connect(item, SIGNAL(sigOkClicked()), this, SLOT(slotFaceWidgetOkClicked()));
                connect(item, SIGNAL(sigCancelClicked()), this, SLOT(slotFaceWidgetCancelClicked()));
                connect(item, SIGNAL(sigDeleteClicked()), this, SLOT(slotFaceWidgetDeleteClicked()));
            }
        }
        updateFaceWidgetCurrentPos();
    }

    //void CompFaceGroupWidget::updateTableTitle()
    //{
    //    if (_obj == nullptr)return;
    //    Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
    //    if (commanger == nullptr)return;

    //    //计算剩余面
    //    QList<int> allPoint = {};
    //    for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
    //        CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
    //        if (otherItem == nullptr)continue;
    //        auto otherObj = commanger->getDataByID(otherItem->data(GeoComObjID).toInt());
    //        if (otherObj == nullptr)continue;
    //        QList<int> ids = otherObj->getMember();
    //        allPoint.append(ids);
    //    }
    //    QStringList header;
    //    header << tr("Default(%1 faces)").arg(6 - allPoint.size());
    //    _ui->tableWidget->setHorizontalHeaderLabels(header);
    //}

    void CompFaceGroupWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);

        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        //设置只能单选
        _ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void CompFaceGroupWidget::setAllFaceGroupSelect(bool type)
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setSelect(false);
        }
    }

    void CompFaceGroupWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setCurrentPos(i, 0);
        }
    }

    void CompFaceGroupWidget::clearGraphHight()
    {
        //退出选择模式
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;
        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        GUI::GUIPickInfo::SetPickInfo(pinfo);
        pickD->clearPickedData();

        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void CompFaceGroupWidget::clearTableWidget()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            //Default面组不可编辑
            if (i == 0)continue;
            QWidget* widget = _ui->tableWidget->cellWidget(i, 0);
            if (widget == nullptr)continue;
            delete widget;
            widget = nullptr;
        }
        _ui->tableWidget->setRowCount(1);
    }

    void CompFaceGroupWidget::updateDefaultFaceGroupIDs()
    {
        if (_parentWidget == nullptr)return;
        CompFaceGroupSelectWidget* widget = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(0, 0));
        if (widget == nullptr) return;
        Interface::FITKGeoComponentManager* commanger = _obj->getShapeAgent()->getGeoComponentManager();
        int objID = widget->data(GeoComObjID).toInt();
        auto com = commanger->getDataByID(objID);
        if (com == nullptr)return;
        QList<int> defaultIDs = _parentWidget->getDefaultFaceGroup(_obj);

        //移除defaultIDs中其他面组的id
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            if (i == 0)continue;
            CompFaceGroupSelectWidget* otherItem = dynamic_cast<CompFaceGroupSelectWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (otherItem == nullptr)continue;
            auto otherObj = commanger->getDataByID(otherItem->data(GeoComObjID).toInt());
            if (otherObj == nullptr)continue;
            QList<int> ids = otherObj->getMember();
            for (auto id : ids)defaultIDs.removeOne(id);
        }
        com->setMember(defaultIDs);

        //更新名称
        QString otherName = com->getDataObjectName();
        if (defaultIDs.size() == 0) {
            otherName += tr("(empty)");
        }
        else {
            otherName += tr("(%1 faces)").arg(defaultIDs.size());
        }
        widget->setName(otherName);
    }
}
