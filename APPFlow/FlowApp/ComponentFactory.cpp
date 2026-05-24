/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**
 * @file ComponentFactory.cpp
 * @brief 组件工厂
 * @author YanZhiHui (chanyuantiandao@126.com)
 * @date 2024-04-19
 */
#include "ComponentFactory.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Component/FITKCompMessageWidget/FITKConsoleComponent.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowInterface.h"
#include "FITK_Component/FITKGeoCompOCC/FITKGeoCompOCCInterface.h"
#include "FITK_Component/FITKMeshGenOF/FITKMeshGenOFInterface.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Component/FITKFlowOFIOHDF5/FITKFlowOFIOHDF5Interface.h"
#include "FITK_Component/FITKOFDictWriter/FITKOFDictWriterIO.h"
#include "FITK_Component/FITKOFMeshIO/FITKOFMeshReader.h"
#include "FITK_Component/FITKGeoOCCIOHDF5/FITKOCCIOHDF5Interface.h"
#include "OperatorsModel/OpersModelInterface.h"
#include "OperatorsGUI/OpersGUIInterface.h"
#include "PreWindowInitializer.h"

QList<AppFrame::FITKComponentInterface *> ComponentFactory::createComponents()
{
    // 自定义组件列表
    QList<AppFrame::FITKComponentInterface *> componentList;
    // 消息窗口组件
    componentList << new Comp::ConsoleComponent(FITKAPP->getGlobalData()->getMainWindow());
    // 3D图形窗口组件
    auto compVTKrender = new Comp::FITKGraph3DWindowInterface;
    componentList << compVTKrender;
    //occ 建模
    componentList << new OCC::FITKGeoCompOCCInterface;
    //OF网格划分
    componentList << new OF::FITKMeshGenOFInterface;
    // 模型数据控制器组件
    componentList << new OperModel::OpersModelInterface;
    // 界面控制器组件
    componentList << new GUIOper::OperatorsGUIInterface;
    // 字典文件写出组件
    componentList << new IO::FITKOFDictWriterIO;
    // OpenFOAM网格读取组件
    componentList << new IO::FITKOFMeshReader;
    // 物理场数据工厂组件
    componentList << new Interface::FITKFlowPhysicsHandlerFactory;
    // hdf5接口组件
    componentList << new IO::FITKFlowOFIOHDF5Interface;
    componentList << new IO::FITKOCCIOHDF5Interface;
    // VTK窗口初始化器
    compVTKrender->addInitializer(1, new PreWindowInitializer);

    // 开启拓扑偏移。
    Comp::FITKGraph3DWindowInterface::SetResolveCoincidentType(Comp::ResolveCoincidentType::RCT_PolygonOffset);

    return componentList;
}
