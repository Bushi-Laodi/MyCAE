/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "WorkBenchHandler.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Kernel/FITKCore/FITKThreadPool.h"

void FlowAppWorkBenchHandler::execHandler()
{
    //文件数量
    const int n = this->getInputFilesCount();
    for (int i = 0; i<n; ++i)
    {
        //遍历文件
        AppFrame::IOFileInfo finfo = this->getInputFileInfo(i);
        if (finfo._suffix.toLower() == "brep" || finfo._suffix.toLower() == "step")
        {
            //导入文件
            this->importGeoFile(QString("%1/%2").arg(finfo._path).arg(finfo._name));
        }
        else if (finfo._suffix.toLower() == "hdf5" && this->isProjectFileMode())
        {
            this->openProjectFile(QString("%1/%2").arg(finfo._path).arg(finfo._name));
        }
 
    }

}

void FlowAppWorkBenchHandler::execOutput()
{
    AppFrame::FITKWorkBenchHandler::execOutput();
    this->saveProjectFile();

}

void FlowAppWorkBenchHandler::importGeoFile(const QString & fileName)
{
    //获取操作器
    Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>("actionImportGeometry");
    if (oper == nullptr) return;
    oper->setArgs("FileName", fileName);
    oper->setArgs("SenderName", "actionImportGeometry");
    //执行操作
    oper->execProfession();
}

void FlowAppWorkBenchHandler::openProjectFile(const QString & fileName)
{
    //获取操作器
    Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>("actionOpen");
    if (oper == nullptr) return;
    oper->setArgs("FileName", fileName); 
    //执行操作
    oper->execProfession();
}

void FlowAppWorkBenchHandler::saveProjectFile()
{
    QString path = this->getOutputPath();
    QString pfile = path + "/AppFlowOFSys.hdf5";
    //创建空的文件
    if (!Core::CreateFile(pfile)) return;
  
    //执行写出
    auto oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>("actionSave");
    if (oper == nullptr) return;
    oper->setArgs("FileName", pfile);
    oper->execProfession();
    //线程池等待
    Core::FITKThreadPool::getInstance()->wait();
}

