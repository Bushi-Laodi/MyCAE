/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "PanelWidgetBase.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QLabel>

namespace GUI
{
    PanelWidgetBase::PanelWidgetBase(MainWindow *mainWindow) : QWidget(mainWindow), m_MainWindow(mainWindow)
    {
        // 创建布局
        m_Layout = new QVBoxLayout(this);
        m_Layout->setMargin(0);
        m_Layout->setSpacing(3);
        // 设置标题
        m_TitleLabel = new QLabel(this);
        m_TitleLabel->setStyleSheet(" QLabel{\
            background-color: qlineargradient(x1 : 0, x2 : 0, y1 : 0, y2 : 1, stop : 0 #f7f7f7, stop:1 #dbdbdb); \
            min-height:32px; \
            max-height:32px; \
            font-size: 12pt; \
            font-weight: bold; \
            color: rgb(57, 65, 89); \
            }");
        m_TitleLabel->setObjectName("PanelTitleLabel");
        m_TitleLabel->setFixedHeight(50);
        m_Layout->addWidget(m_TitleLabel);
    }

    PanelWidgetBase::~PanelWidgetBase()
    {
        // 移除显示部件
//		removeWidget();
        delete m_Layout;
        delete m_TitleLabel;
    }
    // 设置标题内容
    void PanelWidgetBase::setTitle(QString title)
    {
        m_TitleLabel->setText(title);
    }
    // 获取标题内容
    QString PanelWidgetBase::getTitle() const
    {
        return m_TitleLabel->text();
    }
    // 设置标题显示高度
    void PanelWidgetBase::setTitleHeight(int height)
    {
        m_TitleLabel->setFixedHeight(height);
    }
    // 设置显示内容
    void PanelWidgetBase::setWidget(QWidget *widget)
    {
        // 如果已经有内容需要先移除
        this->removeWidget();
        if (widget == nullptr) return;
        // 添加新的内容
        m_Layout->addWidget(widget);
        m_Widget = widget;
    }
    // 移除显示内容
    void PanelWidgetBase::removeWidget()
    {
        if (m_Widget == nullptr) return;
        m_Layout->removeWidget(m_Widget);
        m_Widget->close();
        delete m_Widget;
        m_Widget = nullptr;
    }

    QWidget* PanelWidgetBase::getCurrentWidget()
    {
        return m_Widget;
    }

}
