/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _SIGNAL_PROCESSOR_H___
#define _SIGNAL_PROCESSOR_H___
 
#include <QObject>

namespace AppFrame
{
    class FITKAbstractProgramerDriver;
}

class SignalProcessor : public QObject
{
    Q_OBJECT
public:
    explicit SignalProcessor() = default;
    virtual ~SignalProcessor() = default;


protected slots:
    void on_sendProgramDriverMessageSig(AppFrame::FITKAbstractProgramerDriver* driver, int messageType, const QString& message);

};

#endif
