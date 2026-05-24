/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompSelectComBoxWidget.h"
#include "ui_CompSelectComBoxWidget.h"

#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"

namespace GUI
{
    CompSelectComBoxWidget::CompSelectComBoxWidget(QString type, QWidget * parent):
        QWidget(parent), _type(type)
    {
        _ui = new Ui::CompSelectComBoxWidget();
        _ui->setupUi(this);

        init();
    }

    CompSelectComBoxWidget::~CompSelectComBoxWidget()
    {
        if (_ui)delete _ui;
    }

    void CompSelectComBoxWidget::init()
    {
        _ui->widget_Sub->show();
        _ui->label_Name->setText(_type);
        _ui->pushButton->setCheckable(true);
        _ui->pushButton->setChecked(true);
    }

    void CompSelectComBoxWidget::update()
    {
        if (_myFunction == nullptr)return;
        QString currentOption = _ui->comboBox->currentText();
        if (currentOption.isEmpty())return;
        Core::FITKParameter* subData = _myFunction(currentOption, this);
        if (subData == nullptr)return;

        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_Sub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

        for (auto data : subData->getParameter()) {
            if (data == nullptr)continue;
            QWidget* w = new Core::FITKWidgetComLine(data, this);
            if (w == nullptr)continue;
            _ui->verticalLayout_Sub->addWidget(w);
        }
    }

    void CompSelectComBoxWidget::setFunction(getSubDataFormText function)
    {
        _myFunction = function;
    }

    void CompSelectComBoxWidget::setOptions(QStringList Options)
    {
        _ui->comboBox->blockSignals(true);
        _ui->comboBox->clear();
        _ui->comboBox->addItems(Options);
        _ui->comboBox->setCurrentIndex(0);
        _ui->comboBox->blockSignals(false);
    }

    void CompSelectComBoxWidget::setCurrentText(const QString index)
    {
        _ui->comboBox->blockSignals(true);
        _ui->comboBox->setCurrentText(index);
        _ui->comboBox->blockSignals(false);
    }

    void CompSelectComBoxWidget::setSubWidgetData(Core::FITKParameter* data)
    {
        if (data == nullptr)return;
        //清除全部子界面
        QLayoutItem* item;
        while ((item = _ui->verticalLayout_Sub->takeAt(0)) != nullptr) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater(); // 推荐使用 deleteLater，以确保小部件在适当时机被删除
            }
            delete item; // 删除布局项
        }

        for (auto d : data->getParameter()) {
            if (d == nullptr)continue;
            QWidget* w = new Core::FITKWidgetComLine(d, this);
            if (w == nullptr)continue;
            _ui->verticalLayout_Sub->addWidget(w);
        }
    }

    void CompSelectComBoxWidget::setData(QString name, QVariant data)
    {
        _data.insert(name, data);
    }

    QVariant CompSelectComBoxWidget::getData(const QString name)
    {
        return _data.value(name);
    }

    void CompSelectComBoxWidget::on_pushButton_clicked()
    {
        if (_ui->pushButton->isChecked()) {
            _ui->widget_Sub->show();
        }
        else {
            _ui->widget_Sub->hide();
        }
    }

    void GUI::CompSelectComBoxWidget::on_comboBox_currentIndexChanged(int index)
    {
        Q_UNUSED(index);
        update();
    }
}



