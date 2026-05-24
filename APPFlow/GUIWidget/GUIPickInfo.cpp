/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GUIPickInfo.h"
#include "FITK_Kernel/FITKCore/FITKEnumTransformer.hpp"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKRunTimeSetting.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Interface/FITKInterfaceModel/FITKComponentManager.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#define  ComponentManagerStr "ComponentManagerStr"
#define  PickObjTypeStr      "PickObjType"
#define  PickMethodStr       "PickMethod"
#define  PickParamStr        "PickParam"
#define  CreateComponentStr  "CreateComponent"
#define  ComponentNameStr    "ComponentName"

namespace GUI
{


       /**
     * @brief 清除选择信息
     * 该函数用于清除运行时设置中的与选择信息相关的变量。
     */
    void GUIPickInfo::ClearPickInfo()
    {
        auto RTsetting = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (RTsetting == nullptr) return;
        // 移除运行时设置中的变量
        RTsetting->removeVarient(ComponentManagerStr);
        RTsetting->removeVarient(PickObjTypeStr);
        RTsetting->removeVarient(PickMethodStr);
        RTsetting->removeVarient(PickParamStr);
        RTsetting->removeVarient(CreateComponentStr);
        RTsetting->removeVarient(ComponentNameStr);

        //设置到交互器
        EventOper::GraphInteractionOperator* operPick = Core::FITKOperatorRepo::getInstance()->
            getOperatorT<EventOper::GraphInteractionOperator>("GraphPick");
        if (operPick)
            operPick->setActorStateByPickInfo(GUIPickInfo::PickObjType::POBJNone, GUIPickInfo::PickMethod::PMNone);
    }

    /**
     * @brief 设置选择信息
     * @param info 包含选择信息的结构体
     * 该函数用于设置运行时设置中的与选择信息相关的变量。
     */
    void GUIPickInfo::SetPickInfo(GUIPickInfoStru info, int dataObjId)
    {
        auto RTsetting = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (RTsetting == nullptr) return;
        // 设置运行时设置中的变量值
        RTsetting->setValue(PickObjTypeStr, (int)info._pickObjType);
        RTsetting->setValue(PickMethodStr,  (int)info._pickMethod);
        RTsetting->setValue(PickParamStr,   info._pickParam );
        RTsetting->setValue(CreateComponentStr, info._createComponent);
        RTsetting->setValue(ComponentNameStr, info._componentName);

        //设置到交互器
        EventOper::GraphInteractionOperator* operPick = Core::FITKOperatorRepo::getInstance()->
            getOperatorT<EventOper::GraphInteractionOperator>("GraphPick");
        if(operPick)
            operPick->setActorStateByPickInfo(info._pickObjType, info._pickMethod, dataObjId);
    }

    /**
     * @brief 设置组件管理器
     * @param mgr 组件管理器指针
     * 该函数用于在运行时设置中设置组件管理器的指针。
     */
    void GUIPickInfo::SetComponentManager(Interface::FITKComponentManager* mgr)
    {
        auto RTsetting = FITKAPP->getGlobalData()->getRunTimeSetting();
        if(RTsetting)
            // 设置组件管理器的QVariant表示
            RTsetting->setValue(ComponentManagerStr, mgr->FITKObj2QVarient());
    }

    Interface::FITKComponentManager* GUIPickInfo::GetComponentManager()
    {
        auto RTsetting = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (RTsetting == nullptr) return nullptr;
        QVariant v = RTsetting->getValue(ComponentManagerStr);
        Core::FITKAbstractObject* obj = v.value<Core::FITKAbstractObject*>();
        return dynamic_cast<Interface::FITKComponentManager*>(obj);
    }

    /**
     * @brief 获取选择信息
     * @return GUIPickInfoStru 返回选择信息的结构体
     * 该函数用于从运行时设置中获取与选择信息相关的变量值，并封装到一个结构体中返回。
     */
    GUIPickInfoStru GUIPickInfo::GetPickInfo()
    {
        GUIPickInfoStru pickInfo;

        auto RTsetting = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (RTsetting == nullptr) return pickInfo;
        // 从运行时设置中获取变量值
        pickInfo._pickObjType = (GUIPickInfo::PickObjType)RTsetting->getValueT<int>(PickObjTypeStr);
        pickInfo._pickMethod = (GUIPickInfo::PickMethod)RTsetting->getValueT<int>(PickMethodStr);
        pickInfo._pickParam = RTsetting->getValueT<double>(PickParamStr);
        pickInfo._createComponent = RTsetting->getValueT<bool>(CreateComponentStr);
        pickInfo._componentName = RTsetting->getValueT<QString>(ComponentNameStr);

        return pickInfo;
    }

    /**
     * @brief 根据UI文本获取选择方法
     * @param uiText UI中显示的选择方法文本
     * @return PickMethod 返回解析后的选择方法枚举
     * 该函数用于根据UI文本获取对应的选择方法枚举值。
     */
//     GUI::GUIPickInfo::PickMethod GUIPickInfo::GetPickMethod(QString uiText)
//     {
//         Core::FITKEnumTransfer<PickMethod> pe;
// 
//         bool ok = false;
//         QString text = "PM" + uiText.remove(" ");
//         // 从字符串中解析选择方法枚举
//         return pe.fromString(text,ok);
//     }

}



