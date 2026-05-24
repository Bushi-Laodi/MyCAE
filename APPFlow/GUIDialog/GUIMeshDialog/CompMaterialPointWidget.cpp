/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompMaterialPointWidget.h"
#include "ui_CompMaterialPointWidget.h"

#include <QApplication>
#include <QStyle>

namespace GUI
{
    CompMaterialPointWidget::CompMaterialPointWidget(QWidget * parent):
        QWidget(parent)
    {
        _ui = new Ui::CompMaterialPointWidget();
        _ui->setupUi(this);
        init();
        connect(_ui->pushButton_Delete, SIGNAL(clicked()), this, SIGNAL(sigDeleteClicked()));
        connect(_ui->lineEdit_Point1, SIGNAL(editingFinished()), this, SIGNAL(sigPointChange()));
        connect(_ui->lineEdit_Point2, SIGNAL(editingFinished()), this, SIGNAL(sigPointChange()));
        connect(_ui->lineEdit_Point3, SIGNAL(editingFinished()), this, SIGNAL(sigPointChange()));
    }

    CompMaterialPointWidget::~CompMaterialPointWidget()
    {
        if (_ui)delete _ui;
    }

    void CompMaterialPointWidget::init()
    {
        _currentPos.first = -1;
        _currentPos.second = -1;

        _ui->pushButton_Delete->setStyleSheet("background: transparent;");
        _ui->pushButton_Delete->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    }
    void CompMaterialPointWidget::setName(QString name)
    {
        _ui->label_Name->setText(name);
    }

    QString CompMaterialPointWidget::getName()
    {
        return _ui->label_Name->text();
    }

    void CompMaterialPointWidget::getPoint(double* point)
    {
        point[0] = _ui->lineEdit_Point1->text().toDouble();
        point[1] = _ui->lineEdit_Point2->text().toDouble();
        point[2] = _ui->lineEdit_Point3->text().toDouble();
    }

    void CompMaterialPointWidget::setPoint(double* point)
    {
        _ui->lineEdit_Point1->setText(QString::number(point[0]));
        _ui->lineEdit_Point2->setText(QString::number(point[1]));
        _ui->lineEdit_Point3->setText(QString::number(point[2]));
    }

    void CompMaterialPointWidget::setData(int pos, QVariant value)
    {
        _data.insert(pos, value);
    }

    QVariant CompMaterialPointWidget::data(int pos)
    {
        return _data.value(pos);
    }

    void CompMaterialPointWidget::setCurrentPos(int row, int clo)
    {
        _currentPos.first = row;
        _currentPos.second = clo;
    }

    QPair<int, int> CompMaterialPointWidget::getCurrentPos()
    {
        return _currentPos;
    }
}

