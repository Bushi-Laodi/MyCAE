/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CompFaceGroupSelectWidget.h
 * @brief 面组选择面组件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-08
 * 
 */
#ifndef CompFaceGroupSelectWidget_H
#define CompFaceGroupSelectWidget_H

#include <QWidget>
#include <QLineEdit>

namespace Ui {
    class CompFaceGroupSelectWidget;
}

namespace GUI
{
    /**
     * @brief 面组选择面组件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-08
     */
    class CompFaceGroupSelectWidget :public QWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Face Group Select Widget object
         * @param[i]  parent         My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        CompFaceGroupSelectWidget(QWidget* parent);
        /**
         * @brief Destroy the Comp Face Group Select Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        ~CompFaceGroupSelectWidget();
        /**
         * @brief init
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void init();
        /**
         * @brief Set the Name object
         * @param[i]  name           My Param doc
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setName(QString name);
        /**
         * @brief Get the Name object
         * @return QString name
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        QString getName();
        /**
         * @brief 存储数据
         * @param[i]  pos            数据位置
         * @param[i]  value          数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setData(int pos, QVariant value);
        /**
         * @brief 获取数据
         * @param[i]  pos            数据位置
         * @return QVariant 
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        QVariant data(int pos);
        /**
         * @brief 设置是否处于选择状态
         * @param[i]  type           类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setSelect(bool type);
        /**
         * @brief 设置当前组件处于面组界面中位置
         * @param[i]  row            行
         * @param[i]  clo            列
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void setCurrentPos(int row, int clo);
        /**
         * @brief Get the Current Pos object
         * @return QPair<int, int> 行，列
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        QPair<int, int> getCurrentPos();
        /**
         * @brief    设置是否可以编译
         * @param[i] isEdit    是否可编辑
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        void setIsEdit(bool isEdit);
    signals:
        ;
        /**
         * @brief 编辑名称开始信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void sigEditNameStart();
        /**
         * @brief 编辑名称结束信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void sigEditNameFinish();
        /**
         * @brief 选择模式确定按钮信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void sigOkClicked();
        /**
         * @brief 选择模式取消按钮信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void sigCancelClicked();
        /**
         * @brief 
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        void sigDeleteClicked();
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        Ui::CompFaceGroupSelectWidget* _ui = nullptr;
        /**
         * @brief 数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        QHash<int, QVariant> _data;
        /**
         * @brief 当前位置
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-08
         */
        QPair<int, int> _currentPos;
        /**
         * @brief  是否可编译
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        bool _isEdit = true;
    };
}

#endif
