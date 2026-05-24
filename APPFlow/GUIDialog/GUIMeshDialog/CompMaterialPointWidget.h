/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CompMaterialPointWidget.h
 * @brief 材料点组件界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef CompMaterialPointWidget_H
#define CompMaterialPointWidget_H

#include <QWidget>

namespace Ui {
    class CompMaterialPointWidget;
}

namespace GUI
{
    /**
     * @brief 材料点组件界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class CompMaterialPointWidget :public QWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Material Point Widget object
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompMaterialPointWidget(QWidget* parent);
        /**
         * @brief Destroy the Comp Material Point Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~CompMaterialPointWidget();
        /**
         * @brief 初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 设置组件名称
         * @param[i]  name           名称
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setName(QString name);
        /**
         * @brief 获取组件名称
         * @return QString 组件名称
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QString getName();
        /**
         * @brief 获取点
         * @param[o]  point          材料点
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void getPoint(double* point);
        /**
         * @brief 设置点
         * @param[i]  point          材料点
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setPoint(double* point);
        /**
         * @brief 存储自定义数据
         * @param[i]  pos            数据位置
         * @param[i]  value          数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setData(int pos, QVariant value);
        /**
         * @brief 获取自定义数据
         * @param[i]  pos            数据位置
         * @return QVariant 自定义数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QVariant data(int pos);
        /**
         * @brief 设置当前位置
         * @param[i]  row            行索引
         * @param[i]  clo            列索引
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setCurrentPos(int row, int clo);
        /**
         * @brief 获取当前位置
         * @return QPair<int, int> 行，列 
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QPair<int, int> getCurrentPos();
    signals:
        ;
        /**
         * @brief 删除信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void sigDeleteClicked();
        /**
         * @brief 点数据更改信号
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void sigPointChange();
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::CompMaterialPointWidget* _ui = nullptr;
        /**
         * @brief 自定义数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QHash<int, QVariant> _data;
        /**
         * @brief 当前位置
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QPair<int, int> _currentPos;
    };
}

#endif
