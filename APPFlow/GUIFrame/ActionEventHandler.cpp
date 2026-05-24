/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "ActionEventHandler.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"

namespace GUI
{
    void ActionEventHandler::execOperator()
    {
        QObject* obj = this->sender();
        if (obj == nullptr) return;
        QString objName = obj->objectName();

        Core::FITKActionOperator* actOper = this->getOperator(obj);
        if (actOper != nullptr) actOper->setEmitter(obj);

        if (actOper == nullptr)
        {
            //错误提示
            QString error = QString("%1 Operator create failed !").arg(objName);
            AppFrame::FITKMessageError(error);
        }
        else
            actOper->actionTriggered();
    }

    Core::FITKActionOperator* ActionEventHandler::getOperator(QObject* object)
    {
        Core::FITKActionOperator* oper = nullptr;
        auto                      travelObj = object;
        //递归查找
        while (travelObj != nullptr)
        {
            QString objName = travelObj->objectName();
            //根据名称从仓库查找
            oper = Core::FITKOperatorRepo::getInstance()->
                getOperatorT<Core::FITKActionOperator>(objName);

            
            if (oper != nullptr) return oper;
            
            travelObj = travelObj->parent();
        }

        return oper;
    }

}


