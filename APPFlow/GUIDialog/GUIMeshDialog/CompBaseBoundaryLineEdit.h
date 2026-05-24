/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _CompBaseBoundaryLineEdit_H
#define _CompBaseBoundaryLineEdit_H

#include <QLineEdit>
#include "GUIMeshDialogAPI.h"

namespace GUI
{
    /**
     * @brief 网格划分区域边界组名称组件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI CompBaseBoundaryLineEdit : public QLineEdit
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Base Boundary LineEdit object
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompBaseBoundaryLineEdit(QWidget* parent = nullptr);
        /**
         * @brief Destroy the Comp Base Boundary LineEdit object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual ~CompBaseBoundaryLineEdit();
        /**
         * @brief 设置组件当前位置
         * @param[i]  pos            位置索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setPos(int pos);
        /**
         * @brief 获取位置索引
         * @return int 位置索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        int getPos();
    signals:
        ;
        /**
         * @brief 鼠标移动信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void sigMouseMove();
    protected:
        /**
         * @brief 鼠标移动事件
         * @param[i]  event          事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void mouseMoveEvent(QMouseEvent *event) override;
    private:
        /**
         * @brief 位置索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        int _pos = -1;
    };
}

#endif
