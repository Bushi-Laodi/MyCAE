/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef __OPER_LICENSE_H___
#define __OPER_LICENSE_H___

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace ModelOper
{
    class OperatorsLicense : public  Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        explicit OperatorsLicense() = default;
        virtual ~OperatorsLicense() = default;
    protected:
        bool execGUI() override;
    private:
    };
    Register2FITKOperatorRepo(actionLicense, OperatorsLicense);
}


#endif
