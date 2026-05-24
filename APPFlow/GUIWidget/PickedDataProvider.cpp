/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "PickedDataProvider.h"

// VTK
#include <vtkPlanes.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkAppendFilter.h>

// Graph
#include "FITK_Interface/FITKVTKAlgorithm/FITKGraphActor.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKKeyMouseStates.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// GUI
#include "GUIPickInfo.h"

// Pick
#include "PickedData.h"
#include "PickedDataCalculator.h"

namespace GraphData
{
    // 静态变量初始化。
    PickedDataProvider* PickedDataProvider::s_instance{ nullptr };

    PickedDataProvider* PickedDataProvider::getInstance()
    {
        // 获取实例。
        if (!s_instance)
        {
            s_instance = new PickedDataProvider;
        }

        return s_instance;
    }

    void PickedDataProvider::Delete()
    {
        if (s_instance)
        {
            delete s_instance;
        }

        s_instance = nullptr;
    }

    PickedDataProvider::PickedDataProvider()
    {
        // 初始化键盘监视器。
        m_settings = FITKAPP->getGlobalData()->getKeyMouseStates();

        // 创建拾取可视化数据数据。
        m_dataSetPicked = vtkUnstructuredGrid::New();

        // 创建预选可视化数据。
        m_dataSetPreview = vtkUnstructuredGrid::New();
    }

    PickedDataProvider::~PickedDataProvider()
    {
        clearPickedData();

        // 清除VTK数据。
        if (m_dataSetPicked)
        {
            m_dataSetPicked->Delete();
            m_dataSetPicked = nullptr;
        }

        if (m_dataSetPreview)
        {
            m_dataSetPreview->Delete();
            m_dataSetPreview = nullptr;
        }
    }

    void PickedDataProvider::addDataManually(Interface::FITKModelEnum::FITKModelSetType type, int dataObjId, QList<int> & indice)
    {
        // 创建拾取数据。
        PickedData* data = new PickedData(type, dataObjId, indice);

        // 判断是否有效。
        if (data->isValid())
        {
            m_pickedDataList.push_back(data);
        }
        else
        {
            delete data;
        }

        // 生成拾取数据。
        generatePickedDataSet();
    }

    void PickedDataProvider::clearPickedData()
    {
        // 清除并析构。
        for (PickedData* data : m_pickedDataList)
        {
            if (data)
            {
                data->clearHighlight();
                delete data;
            }
        }

        m_pickedDataList.clear();

        // 同时清空可视化数据。
        if (m_dataSetPicked)
        {
            m_dataSetPicked->Reset();
            m_dataSetPicked->Modified();
        }

        // 同时清空预选数据。
        clearPickedDataPreview();
    }

    void PickedDataProvider::clearPickedDataPreview()
    {
        if (m_dataSetPreview)
        {
            m_dataSetPreview->Reset();
            m_dataSetPreview->Modified();
        }

        if (m_pickedPreviewData)
        {
            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
        }
    }

    vtkDataSet* PickedDataProvider::getPickedPreviewDataSet()
    {
        return m_dataSetPreview;
    }

    vtkDataSet* PickedDataProvider::getPickedDataSet()
    {
        return m_dataSetPicked;
    }

    PickedData* PickedDataProvider::getPickedData(vtkActor* actor, int index, double* pickedWorldPos, bool isPreview)
    {
        // 获取拾取方式及附加信息。
        GUI::GUIPickInfoStru pickInfo = GUI::GUIPickInfo::GetPickInfo();

        // 创建拾取信息。
        PickedData* pickedData = new PickedData(pickInfo, actor, index, pickedWorldPos, isPreview);

        // 判断拾取是否有效。
        if (!pickedData->isValid())
        {
            delete pickedData;
            return nullptr;
        }

        // 创建临时计算器。
        PickedDataCalculator calculator(pickedData);
        calculator.calculate();

        return pickedData;
    }

    PickedData* PickedDataProvider::getPickedData(vtkActor2D* actor, bool isPreview)
    {
        // 获取拾取方式及附加信息。
        GUI::GUIPickInfoStru pickInfo = GUI::GUIPickInfo::GetPickInfo();

        // 创建拾取信息。
        PickedData* pickedData = new PickedData(pickInfo, actor, isPreview);

        // 判断拾取是否有效。
        if (!pickedData->isValid())
        {
            delete pickedData;
            return nullptr;
        }

        // 创建临时计算器。
        PickedDataCalculator calculator(pickedData);
        calculator.calculate();

        return pickedData;
    }

    bool PickedDataProvider::addPreviewPicked(vtkActor* actor, int index, double* pickedWorldPos)
    {
        if (m_pickedPreviewData)
        {
            // 获取拾取方式及附加信息。
            GUI::GUIPickInfoStru pickInfo = GUI::GUIPickInfo::GetPickInfo();

            bool isTheSamePick = false;

            if (m_pickedPreviewData->getPickedInfo()._pickObjType != pickInfo._pickObjType ||
                m_pickedPreviewData->getPickedInfo()._pickMethod != pickInfo._pickMethod ||
                m_pickedPreviewData->getPickedInfo()._pickParam != pickInfo._pickParam)
            {
                // 判断是否与上次预选相同，相同则跳出，节约性能。
                if (m_pickedPreviewData->contains(actor, index))
                {
                    isTheSamePick = true;
                }
            }

            if (isTheSamePick)
            {
                return false;
            }

            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
        }

        // 计算生成拾取数据。
        PickedData* pickedData = getPickedData(actor, index, pickedWorldPos, true);
        if (!pickedData)
        {
            return false;
        }

        m_pickedPreviewData = pickedData;

        // 生成预选数据。
        generatePickedPreviewDataSet();

        return true;
    }

    bool PickedDataProvider::addPreviewPicked(vtkActor2D* actor)
    {
        if (m_pickedPreviewData)
        {
            bool isTheSamePick = false;

            // 判断是否与上次预选相同，相同则跳出，节约性能。
            if (m_pickedPreviewData->contains(actor))
            {
                isTheSamePick = true;
            }

            if (isTheSamePick)
            {
                return false;
            }

            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
        }

        // 计算生成拾取数据。
        PickedData* pickedData = getPickedData(actor, true);
        if (!pickedData)
        {
            return false;
        }

        m_pickedPreviewData = pickedData;

        // 生成预选数据。
        generatePickedPreviewDataSet();

        return true;
    }

    void PickedDataProvider::addPicked(vtkActor* actor, int index, double* pickedWorldPos)
    {
        PickedData* pickedData{ nullptr };

        // 如果有预选数据则使用预选数据。
        if (m_pickedPreviewData)
        {
            pickedData = m_pickedPreviewData->getCopy();

            // 拷贝后析构，防止快速进行多次拾取导致无法预选。
            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
        }
        else
        {
            // 计算生成拾取数据。
            pickedData = getPickedData(actor, index, pickedWorldPos);
        }

        if (!pickedData)
        {
            return;
        }

        // 处理拾取数据。
        dealPickedData(pickedData);

        // 生成拾取数据。
        generatePickedDataSet();

        // 如果有数据被拾取，发送信号。
        if (m_pickedDataList.count() != 0)
        {
            emit sig_dataPicked();
        }
    }

    void PickedDataProvider::addPicked(vtkActor2D* actor)
    {
        PickedData* pickedData{ nullptr };

        // 如果有预选数据则使用预选数据。
        if (m_pickedPreviewData)
        {
            pickedData = m_pickedPreviewData->getCopy();

            // 拷贝后析构，防止快速进行多次拾取导致无法预选。
            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
        }
        else
        {
            // 计算生成拾取数据。
            pickedData = getPickedData(actor);
        }

        if (!pickedData)
        {
            return;
        }

        // 处理拾取数据。
        dealPickedData(pickedData);

        // 生成拾取数据。
        generatePickedDataSet();

        // 如果有数据被拾取，发送信号。
        if (m_pickedDataList.count() != 0)
        {
            emit sig_dataPicked();
        }
    }

    void PickedDataProvider::addPicked(QList<vtkActor*> actors, vtkPlanes* cutPlane)
    {
        // 获取拾取方式及附加信息。
        GUI::GUIPickInfoStru pickInfo = GUI::GUIPickInfo::GetPickInfo();

        // Single拾取模式禁止框选。
        if (pickInfo._pickMethod == GUI::GUIPickInfo::PickMethod::PMSingle)
        {
            return;
        }

        // 是否需要合并或移除数据。
        bool needAddOrSubData = (m_settings->keyPressed(Qt::Key_Shift) && !m_settings->keyPressed(Qt::Key_Control)) ||
            (!m_settings->keyPressed(Qt::Key_Shift) && m_settings->keyPressed(Qt::Key_Control));

        // 不需要操作数据说明是普通框选，需要清空历史拾取信息。
        if (!needAddOrSubData)
        {
            clearPickedData();
        }

        for (vtkActor* actor : actors)
        {
            // 创建拾取信息。
            PickedData* pickedData = new PickedData(pickInfo, actor, cutPlane);

            // 判断拾取是否有效。
            if (!pickedData->isValid())
            {
                delete pickedData;
                continue;
            }

            // 创建临时计算器。
            PickedDataCalculator calculator(pickedData);
            calculator.calculate();

            // 处理拾取数据。
            dealPickedData(pickedData, true);
        }

        // 生成拾取数据。
        generatePickedDataSet();

        // 如果有数据被拾取，发送信号。
        if (m_pickedDataList.count() != 0)
        {
            emit sig_dataPicked();
        }
    }

    void PickedDataProvider::generatePickedPreviewDataSet()
    {
        if (!m_dataSetPreview)
        {
            return;
        }

        m_dataSetPreview->Reset();

        if (!m_pickedPreviewData)
        {
            m_dataSetPreview->Modified();
            return;
        }

        // 为空则不进行计算。
        if (m_pickedPreviewData->isEmpty())
        {
            m_dataSetPreview->Modified();

            delete m_pickedPreviewData;
            m_pickedPreviewData = nullptr;
            return;
        }

        // 获取预选三维数据。
        vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
        m_pickedPreviewData->getDataSet(ugrid);
        m_dataSetPreview->DeepCopy(ugrid);
        ugrid->Delete();
        m_dataSetPreview->Modified();
    }

    void PickedDataProvider::generatePickedDataSet()
    {
        // 合并拾取可视化数据。
        vtkAppendFilter* appendFilter = vtkAppendFilter::New();

        // 析构数据列表。
        QList<vtkUnstructuredGrid*> dataSets;

        // 移除已经被清空的数据。
        for (int i = m_pickedDataList.count() - 1; i >= 0; i--)
        {
            PickedData* data = m_pickedDataList[i];
            if (data->isEmpty())
            {
                m_pickedDataList.removeAt(i);
                delete data;
            }
        }

        for (PickedData* data : m_pickedDataList)
        {
            if (!data)
            {
                continue;
            }

            // 外部创建网格数据，内部进行数据填充。
            vtkUnstructuredGrid* ugrid = vtkUnstructuredGrid::New();
            data->getDataSet(ugrid);
            appendFilter->AddInputData(ugrid);

            dataSets.push_back(ugrid);
        }

        // 合并数据并拷贝。
        appendFilter->Update();

        m_dataSetPicked->DeepCopy(appendFilter->GetOutput());
        m_dataSetPicked->Modified();

        appendFilter->Delete();

        // 移除并析构子数据集。
        for (vtkUnstructuredGrid* dataSet : dataSets)
        {
            if (dataSet)
            {
                dataSet->Delete();
            }
        }

        dataSets.clear();
    }

    void PickedDataProvider::dealPickedData(PickedData* data, bool isAreaPick)
    {
        // 获取拾取方式及附加信息。
        GUI::GUIPickInfoStru pickInfo = GUI::GUIPickInfo::GetPickInfo();

        // 根据当前键盘按键判断数据处理逻辑。
        // Shift按下则进行增量拾取。
        if (m_settings->keyPressed(Qt::Key_Shift) && !m_settings->keyPressed(Qt::Key_Control) &&
            // Single拾取模式不支持增量拾取。
            pickInfo._pickMethod != GUI::GUIPickInfo::PickMethod::PMSingle)
        {
            // 获取相同模型拾取信息进行合并，否则直接添加。
            PickedData* brotherData = getSameModelPickData(data);
            if (brotherData)
            {
                brotherData->add(data);
            }
            else
            {
                m_pickedDataList.push_back(data);
            }
        }
        // Control按下则擦除。
        else if (!m_settings->keyPressed(Qt::Key_Shift) && m_settings->keyPressed(Qt::Key_Control))
        {
            // 获取相同模型拾取信息进行做差，否则直接跳出。
            PickedData* brotherData = getSameModelPickData(data);
            if (brotherData)
            {
                brotherData->subtract(data);
            }
            else
            {
                return;
            }
        }
        // Shift与Control同时按下或均不按下则清空拾取重新选择。
        else
        {
            if (!isAreaPick)
            {
                clearPickedData();
            }

            m_pickedDataList.push_back(data);
        }

        // 移除已经被清空的数据。
        for (int i = m_pickedDataList.count() - 1; i >= 0; i--)
        {
            PickedData* data = m_pickedDataList[i];
            if (data->isEmpty())
            {
                m_pickedDataList.removeAt(i);
                delete data;
            }
        }
    }

    PickedData* PickedDataProvider::getSameModelPickData(PickedData* data)
    {
        if (!data || m_pickedDataList.isEmpty())
        {
            return nullptr;
        }

        // 寻找拾取到的同模型数据。
        for (PickedData* pData : m_pickedDataList)
        {
            if (data->isSameAs(pData))
            {
                return pData;
            }
        }

        return nullptr;
    }

    QList<PickedData*> PickedDataProvider::getPickedList()
    {
        return m_pickedDataList;
    }
}