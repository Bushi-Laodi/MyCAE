/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsOpen.h"
#include <QFile>
#include <QFileDialog>
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h" 
#include "FITK_Kernel/FITKAppFramework/FITKRunTimeSetting.h" 
#include "FITK_Component/FITKFlowOFIOHDF5/FITKFlowOFIOHDF5Interface.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "GUIDialog/GUIMeshDialog/MeshGeoWidget.h"
#include "GUIFrame/PropertyWidget.h"

namespace ModelOper
{
    OperatorsOpen::OperatorsOpen()
    {

    }
    OperatorsOpen::~OperatorsOpen()
    {
    }
    bool OperatorsOpen::execGUI()
    {
        //主界面
        QWidget* mw = FITKAPP->getGlobalData()->getMainWindow();
        //获取工作目录
        QString wk;
        auto settings = FITKAPP->getAppSettings();
        if (settings)
            wk = settings->getWorkingDir();
        // QString fileName = QFileDialog::getOpenFileName(mw, QObject::tr("Open INP File"),"" , "INP(*.inp)");

        QFileDialog dlg(mw, QObject::tr("Open Project File"), wk, "HDF5(*.hdf5)");
        dlg.setAcceptMode(QFileDialog::AcceptOpen);  //打开模式
        dlg.show();
        //阻塞，否则键盘事件处理会出问题
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
        //存储信息
        if (_emitter == nullptr) return false;

        this->setArgs("FileName", fileName);
        return true;
    }
    bool OperatorsOpen::execProfession()
    {
        //文件存在
        QString fileName;
        bool ok = this->argValue<QString>("FileName", fileName);
        if (!ok) return false;
        QFileInfo finfo(fileName);
        if (!finfo.exists()) return false;

        //输出信息
        AppFrame::FITKMessageNormal(QString(tr("Reading project file %1 ...")).arg(fileName));

        ////获取数据对象
        //AbaqusData::FITKAbaqusData* abaData = AbaqusData::FITKAbaqusData::GetDataFromAppFrame();
        //if (!abaData) return false;

        //清理几何数据
        Interface::FITKGeoCommandList* geoCommList = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (!geoCommList)return false;
        geoCommList->clear();
        //清理网格
        Interface::FITKUnstructuredFluidMeshVTK* meshData = FITKAPP->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        if (!meshData)return false;
        meshData->clearMesh();

        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        graphOper->reRender();
        //获取读取组件
        IO::FITKFlowOFIOHDF5Interface* fitkAbaIO = FITKAPP->getComponents()->getComponentTByName<IO::FITKFlowOFIOHDF5Interface>("FITKFlowOFHDF5IO");
        if (fitkAbaIO == nullptr) return false;

        //关联信号，后续处理结束的事件
        connect(fitkAbaIO, SIGNAL(ioThreadFinishedSig()), this, SLOT(ioThreadFinishedSlot()));
        //参数设置
        fitkAbaIO->setFileName(fileName);
        //执行读取，1-读取 2- 写出
        fitkAbaIO->exec(1);
        return true;
    }

    void OperatorsOpen::ioThreadFinishedSlot()
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
            AppFrame::FITKMessageError(QString(tr("Failed reading inp file from %1")).arg(fileName));
            return;
        }
        //读取成功
        AppFrame::FITKMessageNormal(QString(tr("Read project file from %1")).arg(fileName));
        //打开的文件
        AppFrame::FITKRunTimeSetting* rs = FITKAPP->getGlobalData()->getRunTimeSetting();
        if (rs)
            rs->setValue("ProjectFile", fileName);

        //刷新树
        auto modelTreeOper = FITKOPERREPO->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (modelTreeOper)
            modelTreeOper->updateTree();
        //更新几何界面显示
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geoData = geometryData->getDataByIndex(i);
            if (geoData == nullptr)continue;
            graphOper->updateGraph(geoData->getDataObjectID());
        }

        //更新网格
        Interface::FITKUnstructuredFluidMeshVTK* meshData = FITKAPP->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        if (!meshData)return;
        graphOper->updateGraph(meshData->getDataObjectID());

        //更新网格划分区域界面
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;
        GUI::MeshGeoWidget* widget = dynamic_cast<GUI::MeshGeoWidget*>(propertyWidget->getCurrentWidget());
        if (widget)widget->updateWidget();
    }
}

