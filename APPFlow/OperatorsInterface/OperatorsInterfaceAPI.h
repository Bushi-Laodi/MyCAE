/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file   OperatorsInterfaceAPI.h
 * @brief  接口声明
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date   2024-03-27
 */
#ifndef _OperatorsInterfaceAPI_H_
#define _OperatorsInterfaceAPI_H_

#include <QtCore/QtGlobal>


#ifdef OperatorsInterface_API
#define OperatorsInterfaceAPI Q_DECL_EXPORT
#else
#define OperatorsInterfaceAPI Q_DECL_IMPORT
#endif

#endif
