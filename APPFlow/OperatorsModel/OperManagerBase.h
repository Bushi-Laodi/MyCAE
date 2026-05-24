/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @brief 操作器基类
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 */
#ifndef __OPERMANAGERBASE_H__
#define __OPERMANAGERBASE_H__

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "GUIFrame/MainWindow.h"
#include "OperatorsModelAPI.h"
#include <QStringList>

namespace ModelOper
{
    /**
     * @brief 操作器基类
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsModelAPI OperManagerBase : public EventOper::ParaWidgetInterfaceOperator
    {
        Q_OBJECT
    public:
        /**
         * @brief Construct a new Oper Manager Base object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperManagerBase();
        /**
         * @brief Destroy the Oper Manager Base object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual ~OperManagerBase();
    protected:
        /**
         * @brief 参数预处理
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void preArgs() override;
        /**
         * @brief 操作器类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        enum OperType
        {
            None,       ///< 未定义
            Create,     ///< 创建操作
            Edit,       ///< 编辑操作
            Copy,       ///< 拷贝操作
            Delete,     ///< 删除操作
            Rename,     ///< 重命名操作
            Select,     ///< 选择操作
        } _operType{ None };
        /**
         * @brief 主界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUI::MainWindow* _mainWindow = nullptr;
        /**
         * @brief 触发对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QString _senderName = "";
    };
} // namespace ModelOper
#endif //!__OPERMANAGERBASE_H__
