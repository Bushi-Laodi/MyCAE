/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file GUICalculateDialogAPI.h
 * @brief 求解参数对话框接口声明
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-19
 *
 */
#ifndef __GUICalculateDialogAPI_H
#define __GUICalculateDialogAPI_H

#include <QtCore/QtGlobal>

#if defined( GUICalculateDialog_API )
#    define GUICalculateDialogAPI Q_DECL_EXPORT
#else
#    define GUICalculateDialogAPI Q_DECL_IMPORT
#endif

#endif // __GUIWIDGETAPI_H__
