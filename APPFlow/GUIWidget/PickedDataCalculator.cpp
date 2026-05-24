/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "PickedDataCalculator.h"

// VTK
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkPlanes.h>

// APP
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKKeyMouseStates.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

// Global data
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"

// Filter ( Algorithm )
#include "FITK_Interface/FITKVTKAlgorithm/FITKShellFeatureEdges.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKSurfaceFilter.h"
#include "FITK_Interface/FITKVTKAlgorithm/FITKExtractGeometry.h"

// Graph
#include "FITK_Interface/FITKVTKAlgorithm/FITKGraphActor.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKCommons.h"
#include "FITK_Component/FITKFluidVTKGraphAdaptor/FITKFluidVTKGraphObject3D.h"

// Pick
#include "PickedData.h"

namespace GraphData
{
    PickedDataCalculator::PickedDataCalculator(GraphData::PickedData* pickedData) :
        m_pickedData(pickedData)
    {

    }

    void PickedDataCalculator::calculate()
    {
        if (!m_pickedData)
        {
            return;
        }

        // 点选拾取方式
        //@{
        if (m_pickedData->getPickedMouseType() == PickedMouseType::PickedMouseClick)
        {
            // 获取拾取时的方式。
            GUI::GUIPickInfoStru pickInfo = m_pickedData->getPickedInfo();
            switch (pickInfo._pickMethod)
            {
            case GUI::GUIPickInfo::PickMethod::PMIndividually:
            case GUI::GUIPickInfo::PickMethod::PMSingle:
            {
                individually();
                break;
            }
            default:
                return;
            }
        }
        //@}
        // 框选拾取方式
        //@{
        else if (m_pickedData->getPickedMouseType() == PickedMouseType::PickedMouseRubber)
        {
            byAreaPick();
        }
        //@}

        // 不管计算完成都将是否需要计算标识置为false。
        m_pickedData->calculateFinsish();

        // 排序。
        m_pickedData->sortIds();
    }

    void PickedDataCalculator::individually()
    {
        Exchange::FITKFluidVTKGraphObject3D* gobj = m_pickedData->getPickedGraphObejct();
        int index = m_pickedData->getPickedIndex();
        if (!gobj || index < 0)
        {
            return;
        }

        int id = -1;

        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->getPickedDataType())
        {
        case PickedDataType::ModelVertPick:
            // 查找点。
            id = gobj->getShapeIdByVTKCellId(index, Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_VERTEX);
            break;
        case PickedDataType::ModelEdgePick:
            // 查找线。
            id = gobj->getShapeIdByVTKCellId(index, Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_EDGE);
            break;
        case PickedDataType::ModelFacePick:
            // 查找面。
            id = gobj->getShapeIdByVTKCellId(index, Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_FACE);
            break;
        case PickedDataType::ModelSolidPick:
            // 查找体。
            id = gobj->getShapeIdByVTKCellId(index, Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_SOLID);
            break;
        default:
            return;
        }

        if (id == -1)
        {
            return;
        }

        m_pickedData->getPickedIds().push_back(id);
    } 

    void PickedDataCalculator::byAreaPick()
    {
        Exchange::FITKFluidVTKGraphObject3D* gobj = m_pickedData->getPickedGraphObejct();
        vtkPlanes* planes = m_pickedData->getCutPlane();
        vtkActor* actor = m_pickedData->getPickedActor();
        if (!gobj || !planes || !actor)
        {
            return;
        }

        // 获取拾取数据集。
        vtkDataSet* dataSet = actor->GetMapper()->GetInputAsDataSet();
        if (!dataSet)
        {
            return;
        }

        vtkSmartPointer<FITKExtractGeometry> extractor = vtkSmartPointer<FITKExtractGeometry>::New();
        extractor->SetImplicitFunction(planes);
        extractor->SetInputData(dataSet);
        extractor->Update();

        // 通过单元索引获取单元ID。
        const QList<int> cellsIndice = extractor->getSelectOriginalCells();
        if (cellsIndice.isEmpty())
        {
            return;
        }

        int len = 0;        
        Exchange::FITKFluidVTKCommons::ShapeAbsEnum sType;

        // 根据拾取数据类型进行不同数据获取。
        switch (m_pickedData->getPickedDataType())
        {
        case PickedDataType::ModelVertPick:
            len = gobj->getNumberOf(Exchange::FITKFluidVTKCommons::ShapeType::ModelVertex);
            sType = Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_VERTEX;
            break;
        case PickedDataType::ModelEdgePick:
            len = gobj->getNumberOf(Exchange::FITKFluidVTKCommons::ShapeType::ModelEdge);
            sType = Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_EDGE;
            break;
        case PickedDataType::ModelFacePick:
            len = gobj->getNumberOf(Exchange::FITKFluidVTKCommons::ShapeType::ModelFace);
            sType = Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_FACE;
            break;
        case PickedDataType::ModelSolidPick:
            len = gobj->getNumberOf(Exchange::FITKFluidVTKCommons::ShapeType::ModelSolid);
            sType = Exchange::FITKFluidVTKCommons::ShapeAbsEnum::STA_SOLID;
            break;
        default:
            return;
        }

        if (len == 0)
        {
            return;
        }

        // 被拾取形状编号。
        QVector<int> shapeIds;

        // 预处理拾取单元数据。（加速判断拾取子Id包含关系）
        int nCells = dataSet->GetNumberOfCells();
        QVector<int> cellPickedFlags;
        cellPickedFlags.resize(nCells);
        cellPickedFlags.fill(0);

        // 获取形状数据Id。
        for (const int & index : cellsIndice)
        {
            int id = gobj->getShapeIdByVTKCellId(index, sType);
            if (!shapeIds.contains(id))
            {
                shapeIds.push_back(id);
            }

            cellPickedFlags[index] = 1;
        }

        // 保存拾取数据。
        for (int i = 0; i < shapeIds.count(); i++)
        {
            int & shapeId = shapeIds[i];

            // 检测当前OCC数据是否完全被选中。
            QVector<int> subIds = gobj->getVTKCellIdsByShapeId(shapeId, sType);

            bool isFullPicked = true;
            for (const int & id : subIds)
            {
                isFullPicked &= (cellPickedFlags[id] == 1);
            }

            // 完全选中则视为被框选。
            if (isFullPicked)
            {
                m_pickedData->getPickedIds().push_back(shapeId);
            }
        }

        shapeIds.clear();
    }
}