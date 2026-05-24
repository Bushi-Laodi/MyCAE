/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MeshBaseTypeBoxWidget.h"
#include "ui_MeshBaseTypeBoxWidget.h"
#include "MeshBaseWidget.h"
#include "CompBaseBoundaryLineEdit.h"

#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeBox.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeCylinder.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeSphere.h"

#include <QtMath>

namespace GUI
{
    MeshBaseTypeBoxWidget::MeshBaseTypeBoxWidget(QWidget* parent) :
        MeshBaseTypeWidgetBase(parent)
    {
        _ui = new Ui::MeshBaseTypeBoxWidget();
        _ui->setupUi(this);
        //创建预览渲染对象
        _graphObj = new Interface::FITKRegionMeshSizeBox();
        init();

        _ui->pushButton_2->hide();
    }

    MeshBaseTypeBoxWidget::~MeshBaseTypeBoxWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }

        if (_graphObj) {
            delete _graphObj;
            _graphObj = nullptr;
        }

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void MeshBaseTypeBoxWidget::init()
    {
        connect(_ui->lineEdit_BasePoint1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_BasePoint2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_BasePoint3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Dimensions3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Division3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading2, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Grading3, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->label_X0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_X1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_Y0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_Y1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_Z0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->label_Z1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_X0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_X0, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_X1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_X1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Y0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_Y0, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Y1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_Y1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Z0, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_Z0, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        connect(_ui->lineEdit_Z1, SIGNAL(sigMouseMove()), this, SLOT(slotMouseMove()));
        connect(_ui->lineEdit_Z1, SIGNAL(editingFinished()), this, SLOT(slotSaveValue()));
        _ui->label_X0->setPos(0);
        _ui->label_X1->setPos(1);
        _ui->label_Y0->setPos(2);
        _ui->label_Y1->setPos(3);
        _ui->label_Z0->setPos(4);
        _ui->label_Z1->setPos(5);
        _ui->lineEdit_X0->setPos(0);
        _ui->lineEdit_X1->setPos(1);
        _ui->lineEdit_Y0->setPos(2);
        _ui->lineEdit_Y1->setPos(3);
        _ui->lineEdit_Z0->setPos(4);
        _ui->lineEdit_Z1->setPos(5);
    }

    bool MeshBaseTypeBoxWidget::checkValue()
    {
        return true;
    }

    bool MeshBaseTypeBoxWidget::setDataToWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        this->blockSignals(true);

        double basicPoint[3] = { 0,0,0 };
        boxObj->getPoint1(basicPoint);
        _ui->lineEdit_BasePoint1->setText(QString::number(basicPoint[0]));
        _ui->lineEdit_BasePoint2->setText(QString::number(basicPoint[1]));
        _ui->lineEdit_BasePoint3->setText(QString::number(basicPoint[2]));

        double length[3] = { 0,0,0 };
        boxObj->getLength(length);
        _ui->lineEdit_Dimensions1->setText(QString::number(length[0]));
        _ui->lineEdit_Dimensions2->setText(QString::number(length[1]));
        _ui->lineEdit_Dimensions3->setText(QString::number(length[2]));

        _ui->lineEdit_Division1->setText(QString::number(boxObj->getDivision(0)));
        _ui->lineEdit_Division2->setText(QString::number(boxObj->getDivision(1)));
        _ui->lineEdit_Division3->setText(QString::number(boxObj->getDivision(2)));

        _ui->lineEdit_Grading1->setText(QString::number(boxObj->getGrading(0)));
        _ui->lineEdit_Grading2->setText(QString::number(boxObj->getGrading(1)));
        _ui->lineEdit_Grading3->setText(QString::number(boxObj->getGrading(2)));
        QString a = boxObj->getFaceName(0);
        _ui->lineEdit_X0->setText(boxObj->getFaceName(0));
        _ui->lineEdit_X1->setText(boxObj->getFaceName(1));
        _ui->lineEdit_Y0->setText(boxObj->getFaceName(2));
        _ui->lineEdit_Y1->setText(boxObj->getFaceName(3));
        _ui->lineEdit_Z0->setText(boxObj->getFaceName(4));
        _ui->lineEdit_Z1->setText(boxObj->getFaceName(5));

        this->blockSignals(false);
        return true;
    }

    bool MeshBaseTypeBoxWidget::getDataFromWidget(Interface::FITKAbstractRegionMeshSize * obj)
    {
        Interface::FITKRegionMeshSizeBox* boxObj = dynamic_cast<Interface::FITKRegionMeshSizeBox*>(obj);
        if (boxObj == nullptr)return false;

        double basicPoint[3] = { 0,0,0 };
        basicPoint[0] = _ui->lineEdit_BasePoint1->text().toDouble();
        basicPoint[1] = _ui->lineEdit_BasePoint2->text().toDouble();
        basicPoint[2] = _ui->lineEdit_BasePoint3->text().toDouble();
        boxObj->setPoint1(basicPoint);

        double length[3] = { 0,0,0 };
        length[0] = _ui->lineEdit_Dimensions1->text().toDouble();
        length[1] = _ui->lineEdit_Dimensions2->text().toDouble();
        length[2] = _ui->lineEdit_Dimensions3->text().toDouble();
        boxObj->setLength(length);

        boxObj->setDivision(0, _ui->lineEdit_Division1->text().toInt());
        boxObj->setDivision(1, _ui->lineEdit_Division2->text().toInt());
        boxObj->setDivision(2, _ui->lineEdit_Division3->text().toInt());

        boxObj->setGrading(0, _ui->lineEdit_Grading1->text().toDouble());
        boxObj->setGrading(1, _ui->lineEdit_Grading2->text().toDouble());
        boxObj->setGrading(2, _ui->lineEdit_Grading3->text().toDouble());

        boxObj->insertFaceName(0, _ui->lineEdit_X0->text());
        boxObj->insertFaceName(1, _ui->lineEdit_X1->text());
        boxObj->insertFaceName(2, _ui->lineEdit_Y0->text());
        boxObj->insertFaceName(3, _ui->lineEdit_Y1->text());
        boxObj->insertFaceName(4, _ui->lineEdit_Z0->text());
        boxObj->insertFaceName(5, _ui->lineEdit_Z1->text());

        return true;
    }

    void MeshBaseTypeBoxWidget::updateGeometryGraph()
    {
        if (_graphObj == nullptr)return;
        getDataFromWidget(_graphObj);

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(_graphObj->getDataObjectID());
        graphOper->reRender(true);
    }

    void MeshBaseTypeBoxWidget::clearBoundaryBackgroudColor()
    {
        _ui->widget_X0->setStyleSheet("");
        _ui->widget_X1->setStyleSheet("");
        _ui->widget_Y0->setStyleSheet("");
        _ui->widget_Y1->setStyleSheet("");
        _ui->widget_Z0->setStyleSheet("");
        _ui->widget_Z1->setStyleSheet("");
    }

    void MeshBaseTypeBoxWidget::on_pushButton_AutoSize_clicked()
    {
        Interface::FITKGeoCommandList* geoManager = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geoManager == nullptr) return;

        double minPoint[3] = { 9e66,9e66,9e66 };
        double maxPoint[3] = { -9e66, -9e66, -9e66 };
        geoManager->getBoundaryBox(minPoint, maxPoint);

        double xCount = maxPoint[0] - minPoint[0];
        double yCount = maxPoint[1] - minPoint[1];
        double zCount = maxPoint[2] - minPoint[2];
        double rate = qSqrt(xCount*xCount + yCount * yCount + zCount * zCount)*0.005;

        double resultMinPoint[3] = { 0,0,0 };
        resultMinPoint[0] = minPoint[0] - rate;
        resultMinPoint[1] = minPoint[1] - rate;
        resultMinPoint[2] = minPoint[2] - rate;

        double resultMaxPoint[3] = { 0,0,0 };
        resultMaxPoint[0] = maxPoint[0] + rate;
        resultMaxPoint[1] = maxPoint[1] + rate;
        resultMaxPoint[2] = maxPoint[2] + rate;

        double XExtent = resultMaxPoint[0] - resultMinPoint[0];
        double YExtent = resultMaxPoint[1] - resultMinPoint[1];
        double ZExtent = resultMaxPoint[2] - resultMinPoint[2];

        _ui->lineEdit_BasePoint1->setText(QString::number(resultMinPoint[0]));
        _ui->lineEdit_BasePoint2->setText(QString::number(resultMinPoint[1]));
        _ui->lineEdit_BasePoint3->setText(QString::number(resultMinPoint[2]));

        _ui->lineEdit_Dimensions1->setText(QString::number(XExtent));
        _ui->lineEdit_Dimensions2->setText(QString::number(YExtent));
        _ui->lineEdit_Dimensions3->setText(QString::number(ZExtent));

        if (_meshBaseWidget)_meshBaseWidget->saveValue();
        updateGeometryGraph();
    }

    void MeshBaseTypeBoxWidget::slotSaveValue()
    {
        _meshBaseWidget->saveValue();
        updateGeometryGraph();
    }
}


