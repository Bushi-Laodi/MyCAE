/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file GUIWidgetAPI.h
 * @brief 接口宏声明
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-03-05
 */
#ifndef __GUIWIDGETAPI_H__
#define __GUIWIDGETAPI_H__

#include <QtCore/QtGlobal>

#ifdef GUIWidget_API
#define GUIWIDGETAPI Q_DECL_EXPORT
#else
#define GUIWIDGETAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
