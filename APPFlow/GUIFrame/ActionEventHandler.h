/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
*
* @file ActionEventHandler.h
* @brief 处理主界面按钮事件
* @author LiBaojun (libaojunqd@foxmail.com)
* @date 2023-03-28
*
*/
#ifndef __EVENTOPERATOR_H__
#define __EVENTOPERATOR_H__

 
#include <QObject>
 
namespace Core
{
    class FITKActionOperator;
}
 
namespace GUI
{
    class EventOperator;
    /**
     * @brief 处理主界面事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-03-28
     */
    class ActionEventHandler : public QObject
    {
        Q_OBJECT
    public:
        ActionEventHandler() = default;
        ~ActionEventHandler() = default;
        
        
    public slots:
        /**
         * @brief 执行处理操作
         * @author LiBaojun (libaojunqd@foxmail.com)
         * @date 2023-03-28
         */
        void execOperator();

    private:
        /**
         * @brief 获取执行对象
         * @param  object          触发的对象
         * @return Core::FITKActionOperator*
         * @author LiBaojun (libaojunqd@foxmail.com)
         * @date 2023-04-06
         */
        Core::FITKActionOperator* getOperator(QObject* object);

      
    };

}  // namespace EventOper

#endif