/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    GraphDataProviderAPI.h
 * @brief   接口导出宏。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-03-27
 *********************************************************************/

#ifndef __GRAPHDATAPROVIDERAPI_H__
#define __GRAPHDATAPROVIDERAPI_H__

#include <QtCore/QtGlobal>

#include <QDebug>

#if defined( GRAPHDATAPROVIDER_API )
#    define GRAPHDATAPROVIDERAPI Q_DECL_EXPORT
#else
#    define GRAPHDATAPROVIDERAPI Q_DECL_IMPORT
#endif

#endif // __GRAPHDATAPROVIDERAPI_H__
