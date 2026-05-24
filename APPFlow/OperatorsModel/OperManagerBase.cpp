/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperManagerBase.h"
#include "GUIFrame/MainWindow.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKAbstractNDataObject.h"

namespace ModelOper
{
    /**
     * @brief OperManagerBase的构造函数
     *
     * 该构造函数初始化OperManagerBase类的一个实例。它通过动态类型转换从FITKAPP的全局数据中获取GUI::MainWindow的指针，并将其赋值给_mainWindow成员变量。
     *
     * @param args 构造函数参数列表（当前未列出具体参数，表示可能有可选的参数传递给构造函数）
     */
    OperManagerBase::OperManagerBase(/* args */)
    {
        _mainWindow = dynamic_cast<GUI::MainWindow *>(FITKAPP->getGlobalData()->getMainWindow());
    }

    /**
     * @brief OperManagerBase的析构函数
     *
     * 该析构函数释放OperManagerBase类实例的资源，是一个空函数体。
     */
    OperManagerBase::~OperManagerBase()
    {

    }

    void OperManagerBase::preArgs()
    {
        if (_emitter == nullptr)return;

        auto current = _emitter;
        _senderName = current->objectName();
        QString name = _senderName.toLower();
        // 根据对象名称后缀确定操作类型
        if (name.contains("create")) {
            _operType = Create;
        }
        else if (name.contains("edit")) {
            _operType = Edit;
        }
        else if (name.contains("rename")){
            _operType = Rename;
        }
        else if (name.contains("copy")){
            _operType = Copy;
        }
        else if (name.contains("delete")){
            _operType = Delete;
        }
        else if (name.contains("select")) {
            _operType = Select;
        }
    }
} // namespace ModelOper