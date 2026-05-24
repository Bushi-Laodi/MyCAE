/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompTreeItem.h"
#include "ui_CompTreeItem.h"

#include <QTreeWidgetItem>

namespace GUI
{
    CompTreeItem::CompTreeItem(QTreeWidgetItem* itme, QWidget* parent) :
        Core::FITKWidget(parent), _item(itme)
    {
        _ui = new Ui::CompTreeItem();
        _ui->setupUi(this);

        QString name = _item->text(0);
        _ui->label_name->setText(name);
        
        init();

        //图标按钮默认隐藏
        _ui->pushButton_icon->hide();
        connect(_ui->pushButton_icon, SIGNAL(clicked()), this, SIGNAL(sigIconButtonClicked()));
    }

    CompTreeItem::~CompTreeItem()
    {
        if (_ui)delete _ui;
    }

    void CompTreeItem::init()
    {
        _ui->pushButton_icon->setStyleSheet("background: transparent;");
        _ui->pushButton_icon->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
    }

    void CompTreeItem::setButtonIcon(QIcon icon)
    {
        _ui->pushButton_icon->show();
        _ui->pushButton_icon->setIcon(icon);
    }

    void CompTreeItem::setText(QString text)
    {
        _ui->label_name->setText(text);
    }

    QTreeWidgetItem* CompTreeItem::getTreeItem()
    {
        return _item;
    }
}

