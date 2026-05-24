/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompBaseBoundaryLineEdit.h"

namespace GUI {
    CompBaseBoundaryLineEdit::CompBaseBoundaryLineEdit(QWidget* parent):
        QLineEdit(parent)
    {
        //设置鼠标跟踪
        setMouseTracking(true);
    }

    CompBaseBoundaryLineEdit::~CompBaseBoundaryLineEdit()
    {

    }

    void CompBaseBoundaryLineEdit::setPos(int pos)
    {
        _pos = pos;
    }

    int CompBaseBoundaryLineEdit::getPos()
    {
        return _pos;
    }

    void CompBaseBoundaryLineEdit::mouseMoveEvent(QMouseEvent* event)
    {
        emit sigMouseMove();
        QLineEdit::mouseMoveEvent(event);
    }
}
