/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _WidgetOCCEvent_H
#define _WidgetOCCEvent_H

#include "GUIWidgetAPI.h"
#include <QObject>

namespace GraphData {
    class PickedData;
}

namespace GUI
{
    class GUIWIDGETAPI WidgetOCCEvent : public QObject
    {
        Q_OBJECT;
    public:
        WidgetOCCEvent();
        ~WidgetOCCEvent();

        static bool getPoint(GraphData::PickedData* data, double* point);

        static QList<int> getFaces(QList<GraphData::PickedData*> data, bool isOk = true);
    };
}

#endif
