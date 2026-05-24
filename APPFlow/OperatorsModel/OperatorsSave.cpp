/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsSave.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Component/FITKFlowOFIOHDF5/FITKFlowOFIOHDF5Interface.h"
#include "FITK_Kernel/FITKAppFramework/FITKRunTimeSetting.h" 
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include <QFileDialog>

namespace ModelOper
{
    OperatorsSave::OperatorsSave()
    {

    }

    OperatorsSave::~OperatorsSave()
    {

    }

    bool OperatorsSave::execGUI()
    {
        //从运行设置中获取保存的文件
        AppFrame::FITKRunTimeSetting* rs = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (rs)
        {
            QString file = rs->getValueT<QString>("ProjectFile");
            if (QFile::exists(file))
            {
                this->setArgs("FileName", file);
                return true;
            }
        }

        //选择文件
        this->clearArgs();
        //主界面
        QWidget* mw = FITKAPP->getGlobalData()->getMainWindow();
        //获取工作目录
        QString wk;
        AppFrame::FITKAppSettings* settings = FITKAPP->getAppSettings();
        if (settings)
            wk = settings->getWorkingDir();
        //        _fileName = QFileDialog::getSaveFileName(mw, QObject::tr("Write INP File"), wk, "INP(*.inp)");
        QFileDialog dlg(mw, QObject::tr("Save Project File"), wk, "HDF5(*.hdf5)");
        dlg.setAcceptMode(QFileDialog::AcceptSave);  //保存模式
        dlg.show();
        //阻塞，否则键盘按键事件不能处理
        bool accept = false;
        QEventLoop loop;
        connect(&dlg, &QFileDialog::accepted, [&] {loop.quit(); accept = true;  });
        connect(&dlg, &QFileDialog::rejected, [&] {loop.quit(); accept = false;  });
        loop.exec();
        if (!accept) return false;

        QStringList files = dlg.selectedFiles(); //选择的文件名称
        if (files.isEmpty()) return false;
        QString fileName = files.at(0);
        if (fileName.isEmpty())
            return false;
        if (!fileName.toLower().endsWith(".hdf5"))
            fileName += ".hdf5";
        this->setArgs("FileName", fileName);
        return true;
    }

    bool OperatorsSave::execProfession()
    {
        IO::FITKFlowOFIOHDF5Interface* fitkAbaIO = FITKAPP->getComponents()->getComponentTByName<IO::FITKFlowOFIOHDF5Interface>("FITKFlowOFHDF5IO");
        if (fitkAbaIO == nullptr) return false;
        //关联信号，后续处理结束的事件
        connect(fitkAbaIO, SIGNAL(ioThreadFinishedSig()), this, SLOT(ioThreadFinishedSlot()));
        QString fileName;
        bool ok = this->argValue<QString>("FileName", fileName);
        //参数设置
        fitkAbaIO->setFileName(fileName);

        //执行读取，1-读取 2- 写出
        fitkAbaIO->exec(2);
        return true;
    }

    void OperatorsSave::ioThreadFinishedSlot()
    {
        QString fileName;
        bool ok = this->argValue<QString>("FileName", fileName);
        this->clearArgs();

        auto fitkAbaIO = FITKAPP->getComponents()->getComponentTByName<IO::FITKFlowOFIOHDF5Interface>("FITKFlowOFHDF5IO");
        if (fitkAbaIO == nullptr) return;
        //执行结束打断信号，避免下次执行重复关联，以及其他读写线程触发该槽函数
        disconnect(fitkAbaIO, SIGNAL(ioThreadFinishedSig()), this, SLOT(ioThreadFinishedSlot()));
        //是否读取成功
        const bool success = fitkAbaIO->isSuccess();
        if (!success)
        {
            //读取失败
            AppFrame::FITKMessageError(QString(tr("Failed writing project file to %1")).arg(fileName));

        }
        else
        {
            AppFrame::FITKMessageNormal(QString(tr("Write project file to %1")).arg(fileName));
            //保存的文件
            AppFrame::FITKRunTimeSetting* rs = FITKAPP->getGlobalData()->getRunTimeSetting();
            if (rs)
                rs->setValue("ProjectFile", fileName);
        }
    }
}

