/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file GUIMeshDialogAPI.h
 * @brief 几何相关对话框接口声明
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-19
 *
 */
#ifndef _GUIMeshDialogAPI_H
#define _GUIMeshDialogAPI_H

#include <QtCore/QtGlobal>

#if defined( GUIMeshDialog_API )
#    define GUIMeshDialogAPI Q_DECL_EXPORT
#else
#    define GUIMeshDialogAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
