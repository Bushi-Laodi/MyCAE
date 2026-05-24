/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _OperatorsNew_H
#define _OperatorsNew_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsNew :public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        OperatorsNew();
        ~OperatorsNew();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionNew, OperatorsNew);
}

#endif