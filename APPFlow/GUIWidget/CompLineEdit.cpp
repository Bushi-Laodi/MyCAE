/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "CompLineEdit.h"

#include <QKeyEvent>

namespace GUI
{
    CompLineEdit::CompLineEdit(QWidget * widget):
        QLineEdit(widget)
    {
        setReadOnly(true);
        setStyleSheet("background: transparent;");
    }

    CompLineEdit::~CompLineEdit()
    {
        
    }

    void CompLineEdit::mouseDoubleClickEvent(QMouseEvent* event)
    {
        Q_UNUSED(event);

        //QLineEdit::mouseDoubleClickEvent(event);
        setReadOnly(false);
        setStyleSheet("");
        emit sigEditStart();
        _isEdit = true;
    }

    void CompLineEdit::focusOutEvent(QFocusEvent * event)
    {
        if (_isEdit) {
            QLineEdit::focusOutEvent(event);
            setReadOnly(true);
            setStyleSheet("background: transparent;");
            emit sigEditFinish();
            _isEdit = false;
        }
    }

    void CompLineEdit::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            if (_isEdit) {
                QLineEdit::keyPressEvent(event);
                setReadOnly(true);
                setStyleSheet("background: transparent;");
                emit sigEditFinish();
                _isEdit = false;
            }
        }
        else {
            QLineEdit::keyPressEvent(event);
        }
    }
}
