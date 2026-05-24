/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompFaceGroupSelectWidget.h"
#include "ui_CompFaceGroupSelectWidget.h"

#include <QApplication>
#include <QStyle>

namespace GUI
{
    GUI::CompFaceGroupSelectWidget::CompFaceGroupSelectWidget(QWidget * parent):
        QWidget(parent)
    {
        _ui = new Ui::CompFaceGroupSelectWidget();
        _ui->setupUi(this);

        init();
        setIsEdit(true);
    }

    GUI::CompFaceGroupSelectWidget::~CompFaceGroupSelectWidget()
    {
        if (_ui)delete _ui;
        _data.clear();
    }

    void CompFaceGroupSelectWidget::init()
    {
        _currentPos.first = -1;
        _currentPos.second = -1;

        _ui->pushButton_OK->setStyleSheet("background: transparent;");
        _ui->pushButton_OK->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
        _ui->pushButton_OK->hide();

        _ui->pushButton_Cancel->setStyleSheet("background: transparent;");
        _ui->pushButton_Cancel->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogBack));
        //_ui->pushButton_Cancel->setIcon(QIcon(":/icons/back.png"));
        _ui->pushButton_Cancel->hide();

        _ui->pushButton_Delete->setStyleSheet("background: transparent;");
        _ui->pushButton_Delete->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
    }

    void CompFaceGroupSelectWidget::setName(QString name)
    {
        _ui->lineEdit_Name->setText(name);
    }

    QString CompFaceGroupSelectWidget::getName()
    {
        return _ui->lineEdit_Name->text();
    }

    void CompFaceGroupSelectWidget::setData(int pos, QVariant value)
    {
        _data.insert(pos, value);
    }

    QVariant CompFaceGroupSelectWidget::data(int pos)
    {
        return _data.value(pos);
    }
    void CompFaceGroupSelectWidget::setSelect(bool type)
    {
        if (type) {
            _ui->pushButton_OK->show();
            _ui->pushButton_Cancel->show();
        }
        else
        {
            _ui->pushButton_OK->hide();
            _ui->pushButton_Cancel->hide();
        }
    }
    void CompFaceGroupSelectWidget::setCurrentPos(int row, int clo)
    {
        _currentPos.first = row;
        _currentPos.second = clo;
    }
    QPair<int, int> CompFaceGroupSelectWidget::getCurrentPos()
    {
        return _currentPos;
    }

    void CompFaceGroupSelectWidget::setIsEdit(bool isEdit)
    {
        _isEdit = isEdit;
        if (_isEdit) {
            this->setEnabled(true);
            _ui->pushButton_Delete->show();
            connect(_ui->lineEdit_Name, SIGNAL(sigEditStart()), this, SIGNAL(sigEditNameStart()));
            connect(_ui->lineEdit_Name, SIGNAL(sigEditFinish()), this, SIGNAL(sigEditNameFinish()));
            connect(_ui->pushButton_OK, SIGNAL(clicked()), this, SIGNAL(sigOkClicked()));
            connect(_ui->pushButton_Cancel, SIGNAL(clicked()), this, SIGNAL(sigCancelClicked()));
            connect(_ui->pushButton_Delete, SIGNAL(clicked()), this, SIGNAL(sigDeleteClicked()));
        }
        else {
            this->setEnabled(false);
            _ui->pushButton_Delete->hide();
            disconnect(_ui->lineEdit_Name, SIGNAL(sigEditStart()), this, SIGNAL(sigEditNameStart()));
            disconnect(_ui->lineEdit_Name, SIGNAL(sigEditFinish()), this, SIGNAL(sigEditNameFinish()));
            disconnect(_ui->pushButton_OK, SIGNAL(clicked()), this, SIGNAL(sigOkClicked()));
            disconnect(_ui->pushButton_Cancel, SIGNAL(clicked()), this, SIGNAL(sigCancelClicked()));
            disconnect(_ui->pushButton_Delete, SIGNAL(clicked()), this, SIGNAL(sigDeleteClicked()));
        }
    }
}

