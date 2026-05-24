/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CompLineEdit.h
 * @brief 编辑框组件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-16
 * 
 */
#ifndef CompLineEdit_H
#define CompLineEdit_H

#include "GUIWidgetAPI.h"
#include <QLineEdit>

namespace GUI
{
    /**
     * @brief 编辑框组件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-16
     */
    class GUIWIDGETAPI CompLineEdit : public QLineEdit
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Line Edit object
         * @param[i]  widget         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        CompLineEdit(QWidget* widget);
        /**
         * @brief Destroy the Comp Line Edit object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        ~CompLineEdit();
    signals:
        ;
        /**
         * @brief 编辑开始信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void sigEditStart();
        /**
         * @brief 编辑完成信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void sigEditFinish();
    private:
        /**
         * @brief 鼠标双击事件重写
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        /**
         * @brief 失焦事件重写
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void focusOutEvent(QFocusEvent* event) override;
        /**
         * @brief 回车事件
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void keyPressEvent(QKeyEvent* event) override;
    private:
        /**
         * @brief 是否编辑标记位
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        bool _isEdit = false;
    };
}

#endif
