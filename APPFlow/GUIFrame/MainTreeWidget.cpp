/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "MainTreeWidget.h"

#include "GUIWidget/TreeWidget.h"
#include <QTreeWidgetItem>

namespace GUI
{
    MainTreeWidget::MainTreeWidget(MainWindow* parent) :
        PanelWidgetBase(parent)
    {
        this->setTitle(tr("Tree"));

        _treeWidget = new TreeWidget(this);
        this->setWidget(_treeWidget);

        init();
    }

    MainTreeWidget::~MainTreeWidget()
    {
        if (_treeWidget)delete _treeWidget;
    }

    void MainTreeWidget::init()
    {
        _treeWidget->updateTree();
    }

    TreeWidget * MainTreeWidget::getTreeWidget()
    {
        return _treeWidget;
    }
}
