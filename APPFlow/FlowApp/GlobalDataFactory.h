/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file GlobalDataFactory.h
 * @brief 全局数据工厂
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19 13:59
 */
#ifndef __GLOBALDATAFACTORY__
#define __GLOBALDATAFACTORY__

#include "FITK_Kernel/FITKAppFramework/FITKAbstractGlobalDataFactory.h"
/**
 * @brief 全局数据工厂类
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
class GlobalDataFactory : public AppFrame::FITKAbstractGlobalDataFactory
{
private:
    /**
     * @brief 测试的函数
     * @return Core::FITKAbstractDataObject*
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2024-03-02
     */
    virtual Core::FITKAbstractDataObject *createMeshData() override;
    /**
     * @brief 创建几何数据
     * @return 几何数据对象
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    virtual Core::FITKAbstractDataObject *createGeoData() override;
    /**
     * @brief 创建物理场数据
     * @return 物理场数据
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    virtual Core::FITKAbstractDataObject *createPhysicsData() override;
    /**
     * @brief 创建后处理数据
     * @return 后处理场数据
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    virtual Core::FITKAbstractDataObject *createPostData() override;
    /**
     * @brief 创建其他数据
     * @return 其他数据
     * @author YanZhiHui (chanyuantiandao@126.com)
     * @date 2024-04-19
     */
    virtual QHash<int, Core::FITKAbstractDataObject *> createOtherData() override;
};

#endif
