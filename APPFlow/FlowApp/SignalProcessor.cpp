/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "SignalProcessor.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"

void SignalProcessor::on_sendProgramDriverMessageSig(AppFrame::FITKAbstractProgramerDriver* driver, int messageType, const QString& message)
{
    AppFrame::FITKMessageInfo(message);
}

