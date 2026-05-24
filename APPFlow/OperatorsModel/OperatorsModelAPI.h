/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 *
 * @file OperatorsModelAPI.h
 * @brief  声明接口
 * @author LiBaojun (libaojunqd@foxmail.com)
 * @date 2024-04-14
 *
 */
#ifndef _OPERATORMODELS___API_H___
#define _OPERATORMODELS___API_H___

#include <QtCore/QtGlobal>

#ifdef OperatorsModel_API
#define OperatorsModelAPI Q_DECL_EXPORT
#else
#define OperatorsModelAPI Q_DECL_IMPORT
#endif
#endif
