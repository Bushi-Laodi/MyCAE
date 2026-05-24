/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    OperatorsGUIAPI.h
 * @brief   For export macro.
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef _OperatorsGUIAPI_H_
#define _OperatorsGUIAPI_H_

#include <QtCore/QtGlobal>

#include <QDebug>

#if defined(OperatorsGUI_API)
#define OperatorsGUIAPI Q_DECL_EXPORT
#else
#define OperatorsGUIAPI Q_DECL_IMPORT
#endif

#endif
