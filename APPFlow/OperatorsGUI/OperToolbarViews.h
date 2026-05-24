/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * 
 * @file OperToolbarViews.h
 * @brief 工具栏视角事件操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-08
 * 
 */
#ifndef _OPERTOOLBARVIEWS_H__
#define _OPERTOOLBARVIEWS_H__

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace GUIOper
{
    /**
     * @brief 参数
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-08
     */
    struct CameraPara
    {
        QString _type = "";
        double _focalPoint[3] = { 0 };
        double _viewUp[3] = { 0 };
        double _position[3] = { 0 };
    };
    /**
     * @brief 工具栏视角事件操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-08
     */
    class OperToolbarViews : public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        /**
         * @brief 视角类型枚举
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        enum ViewDir
        {
            Front = 0,
            Back,
            Top,
            Bottom,
            Left,
            Right,
            Isometric,
            User1,
            User2,
            User3
        };
        Q_ENUM(ViewDir);
        /**
         * @brief Construct a new Oper Toolbar Views object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        explicit OperToolbarViews();
        /**
         * @brief Destroy the Oper Toolbar Views object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        ~OperToolbarViews( )         = default;
    private:
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        void init();
        /**
         * @brief ui事件
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-08
         */
        bool execGUI( ) override;
        /**
         * @brief 获取视角下的相机设定参数
         * @param  d                固定视角类型
         * @param  focalPoint       相机焦点
         * @param  viewupDir        相机向上朝向
         * @param  cPos             相机位置
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-08
         */
        bool getCameraPara(ViewDir d, double* focalPoint, double* viewupDir, double* cPos);
        /**
         * @brief 配置文件中保存视角参数
         * @param[i]  camPara        视角参数
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-05-30
         */
        bool saveValue(CameraPara* camPara);
    };

    //根据按钮的ObjectName进行注册按钮事件
    Register2FITKOperatorRepo(actionViewFront, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewBack, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewTop, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewBottom, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewLeft, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewRight, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewIso, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewAuto, OperToolbarViews);
    Register2FITKOperatorRepo(actionSaveImage, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewParallel, OperToolbarViews);
    Register2FITKOperatorRepo(actionViewPerspective, OperToolbarViews);

}  // namespace GUIOper

#endif