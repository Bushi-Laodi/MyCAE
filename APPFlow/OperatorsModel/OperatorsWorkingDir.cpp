/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsWorkingDir.h"

#include "FITK_Component/FITKWidget/FITKWorkingDirDialog.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    bool OperatorsWorkingDir::execGUI()
    {
        //错误值
        if (_emitter == nullptr) return false;

        const QString sName = _emitter->objectName();
        if (sName == "actionWorkingDir")
        {
            //弹出对话框
            QDialog* dlg = new Comp::FITKWorkingDirDialog(FITKAPP->getGlobalData()->getMainWindow());
            dlg->show();
            return false;
        }

        return false;
    }
}

