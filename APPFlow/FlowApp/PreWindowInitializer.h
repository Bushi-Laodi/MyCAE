/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/*****************************************************************//**
 * @file    PreWindowInitializer.h
 * @brief   前处理窗口初始化。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-04-17
 *********************************************************************/

#ifndef __PRE_WIDOWINIINTIALIZER_H___
#define __PRE_WIDOWINIINTIALIZER_H___

#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowInterface.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraphInteractionStyle.h"

#include <QTimer>

// 前置声明
class vtkRenderer;
class vtkRenderWindow;

namespace EventOper
{
    class GraphInteractionOperator;
    class GraphEventOperator;
};

namespace Comp
{
    class FITKGraph3DWindowVTK;
};

/**
 * @brief   前处理窗口初始化。
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-04-17
 */
class PreWindowInitializer : public Comp::Graph3DWindowInitializer
{
public:
    /**
     * @brief   构造函数。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    explicit PreWindowInitializer();

    /**
     * @brief   析构函数。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    virtual ~PreWindowInitializer() = default;

    /**
     * @brief   获取交互方式。[重写]
     * @return  交互器交互方式
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    virtual Comp::FITKGraphInteractionStyle* getStyle() override;

protected:

private:

};


class PreWindowInteractionStyle : public  Comp::FITKGraphInteractionStyle
{
    Q_OBJECT
public:
    /**
     * @brief   创建实例。[静态]
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    static PreWindowInteractionStyle* New();

    vtkTypeMacro(PreWindowInteractionStyle,FITKGraphInteractionStyle);

private:
    /**
        * @brief 鼠标左键按下事件响应
        * @author LiBaojun (libaojunqd@foxmail.com)
        * @date 2023-07-25
        */
    virtual void OnLeftButtonDown() override;

    /**
     * @brief 鼠标左键抬起事件响应
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    virtual void OnLeftButtonUp() override;
    /**
     * @brief 鼠标中键按下事件响应
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-07-25
     */
    virtual void OnMiddleButtonDown() override;
    /**
     * @brief 鼠标中键抬起事件响应
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    virtual void OnMiddleButtonUp() override;
    /**
     * @brief 鼠标移动事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-03
     */
    virtual void OnMouseMove() override;

    /**
     * @brief 鼠标滑轮事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    void OnMouseWheelForward() override;
    /**
     * @brief 鼠标滑轮事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    void OnMouseWheelBackward() override;
    /**
     * @brief 鼠标中键点击事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    void OnRightButtonDown() override;
    /**
     * @brief 鼠标中键释放事件
     * @author LiBaojun (libaojunqd@foxmail.com)
     * @date 2023-11-04
     */
    void OnRightButtonUp() override;

    /**
     * @brief   框选。
     * @param   startPos：鼠标起始坐标
     * @param   endPos：鼠标结束坐标
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-18
     */
    void areaPick(int* startPos, int* endPos) override;

private:
    /**
     * @brief   构造函数。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    PreWindowInteractionStyle();

    /**
     * @brief   析构函数。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    ~PreWindowInteractionStyle() = default;

    /**
     * @brief   获取当前三维窗口
     * @return  三维窗口
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    Comp::FITKGraph3DWindowVTK* getCurrentGraphWindow();

    /**
     * @brief   在当前鼠标位置执行一次点击拾取操作。
     * @param   isPreview：是否为预选[缺省]
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    void pick(bool isPreview = false);

    /**
     * @brief   在当前鼠标位置执行一次点击拾取节点操作。（预留）
     * @param   graphWindow：可视化窗口
     * @param   pos：拾取位置
     * @param   isPreview：是否为预选
     * @param   tol：拾取精度容差[缺省]
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    void pickPoint(Comp::FITKGraph3DWindowVTK* graphWindow, int* pos, bool isPreview, double tol = 0.005);

    /**
     * @brief   在当前鼠标位置执行一次点击拾取单元操作。（点、线、面、体拾取均使用此方法）
     * @param   graphWindow：可视化窗口
     * @param   pos：拾取位置
     * @param   isPreview：是否为预选
     * @param   tol：拾取精度容差[缺省]
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    void pickCell(Comp::FITKGraph3DWindowVTK* graphWindow, int* pos, bool isPreview, double tol = 0.003);

private slots:
    /**
     * @brief   槽函数 - 预选高亮定时器槽函数，执行预选高亮。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    void slot_prePick();

private:
    /**
     * @brief   预选高亮定时器，悬浮200ms后显示预选拾取对象。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    QTimer m_timerForPrePick;

    /**
     * @brief   预选高亮可视化操作器。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    EventOper::GraphInteractionOperator* m_operPreview{ nullptr };

    /**
     * @brief   高亮可视化操作器。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    EventOper::GraphInteractionOperator* m_operPick{ nullptr };

    /**
     * @brief   前处理可视化操作器。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-04-17
     */
    EventOper::GraphEventOperator* m_operGraph{ nullptr }; 

};
#endif