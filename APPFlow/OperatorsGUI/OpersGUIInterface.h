/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   OpersGUIInterface.h
 * @brief  GUI操作器的接口调用，无实际作用
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-03-26
 */
#ifndef __OPERSGUIINTERFACE_H__
#define __OPERSGUIINTERFACE_H__

#include "OperatorsGUIAPI.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponentInterface.h"


namespace GUIOper
{
    class OperatorsGUIAPI OperatorsGUIInterface : public AppFrame::FITKComponentInterface
    {
    public:
        explicit OperatorsGUIInterface() = default;
        virtual ~OperatorsGUIInterface() = default;

        void init() override;
        /**
         * @brief  获取部件名称
         * @return 部件名称
         * @author YanZhiHui (chanyuantiandao@126.com)
         * @data   2024-03-26
         */
        virtual QString getComponentName() override;

    };


}


#endif
