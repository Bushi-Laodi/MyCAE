/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file CompFaceGroupWidget.h
 * @brief 面组界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _CompFaceGroupWidget_H
#define _CompFaceGroupWidget_H

#include "GUIWidget/GUIWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class CompFaceGroupWidget;
}

namespace Interface {
    class FITKAbsGeoCommand;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GeometryWidgetBase;
    /**
     * @brief 面组界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class CompFaceGroupWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Face Group Widget object
         * @param[i]  paraent        父对象
         * @param[i]  obj            几何数据对象
         * @param[i]  oper           操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        CompFaceGroupWidget(QWidget* paraent, Interface::FITKAbsGeoCommand* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Comp Face Group Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~CompFaceGroupWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 设置面组
         * @param[i]  rowIndex       面组所对应的行数
         * @param[i]  facesId        面id组
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setFaceGroupValue(int rowIndex, QList<int> facesId, bool removefromOthers = true);
        /**
         * @brief 获取当前几何数据对象
         * @return Interface::FITKAbsGeoCommand* 几何数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
        /**
         * @brief Set the Data To Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
        /**
         * @brief 清除所有高亮
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearGraphHight();
    private slots:
        ;
        /**
         * @brief tableWidget clear slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Clear_clicked();
        /**
         * @brief tableWidget add slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Add_clicked();
        /**
         * @brief 表格点击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotCellTableClicked(int row, int column);
        /**
         * @brief 面组名称修改事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotEditNameStart();
        /**
         * @brief face group rename finish slot
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotEditNameFinish();
        /**
         * @brief 面组界面ok点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetOkClicked();
        /**
         * @brief 面组界面cancel点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetCancelClicked();
        /**
         * @brief 面组界面delete点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotFaceWidgetDeleteClicked();
    private:
        /**
         * @brief check tableWidget value
         * @return true success
         * @return false fail
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool checkValue();
        //更新表格标题
        //void updateTableTitle();
        /**
         * @brief init table Widget
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void initTableWidget();
        /**
         * @brief Set the All Face Group isSelect
         * @param[i]  type           isSelect
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setAllFaceGroupSelect(bool type);
        /**
         * @brief 刷新面组界面记录的位置
         * （为解决点击界面控件时，QTableWidget未触发不知道当前界面所在的位置问题）
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateFaceWidgetCurrentPos();
        /**
         * @brief 清除Table
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearTableWidget();
        /**
         * @brief    更新Default面组ID
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-31
         */
        void updateDefaultFaceGroupIDs();
    private:
        /**
         * @brief geometry data object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKAbsGeoCommand* _obj = nullptr;
        /**
         * @brief ui object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::CompFaceGroupWidget* _ui = nullptr;
        /**
         * @brief operators object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief  父界面兑现
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-31
         */
        GeometryWidgetBase* _parentWidget = nullptr;
    };
}

#endif
