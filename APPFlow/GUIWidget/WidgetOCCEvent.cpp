/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "WidgetOCCEvent.h"
#include "PickedData.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKDataRepo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
//#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFHexMeshBaseMeshBox.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeomTools.h"
#include "FITK_Component/FITKGeoCompOCC/FITKAbstractOCCModel.h"
#include "FITK_Component/FITKGeoCompOCC/FITKOCCModelSimpleShape.h"

//#include <TopoDS_Vertex.hxx>
//#include <TopoDS.hxx>
//#include <BRep_Tool.hxx>

namespace GUI
{
    WidgetOCCEvent::WidgetOCCEvent()
    {

    }

    WidgetOCCEvent::~WidgetOCCEvent()
    {

    }

    bool WidgetOCCEvent::getPoint(GraphData::PickedData* data, double* point)
    {
        QList<int> ids = data->getPickedIds();
        if (ids.isEmpty()) {
            return false;
        }

        Interface::FITKInterfaceGeometryFactory* factory = Interface::FITKInterfaceGeometryFactory::getInstance();
        if (factory == nullptr)return false;
        Interface::FITKAbstractGeomToolsCreator* creator =factory->getGeomToolsCreator();
        if (creator == nullptr)return false;
        Interface::FITKAbstractGeomPointTool* pointTool = creator->createPointTool();
        if (pointTool == nullptr)return false;
        return pointTool->getXYZByID(ids[0], point);

        //int DataObjId = data->getPickedDataObjId();
        //GraphData::PickedDataType type = data->getPickedDataType();
        //if (ids.size() == 0) {
        //    isOk = false;
        //    return;
        //}

        ////数据仓库中获取数据
        //Interface::FITKAbsGeoCommand* model = Core::FITKDataRepo::getInstance()->getTDataByID<Interface::FITKAbsGeoCommand>(DataObjId);
        //if (model->getShapeAgent() == nullptr) {
        //    isOk = false;
        //    return;
        //}

        //TopoDS_Shape shape;
        //OCC::FITKAbstractOCCModel* OCCModel = dynamic_cast<OCC::FITKAbstractOCCModel*>(model->getShapeAgent());
        //if (OCCModel == nullptr) {
        //    isOk = false;
        //    return;
        //}
        //shape = OCCModel->getShape(Interface::FITKModelEnum::FMSPoint, ids[0]);

        //if (shape.IsNull()) {
        //    isOk = false;
        //    return;
        //}

        //TopoDS_Vertex vertex = TopoDS::Vertex(shape);
        //gp_Pnt pt = BRep_Tool::Pnt(vertex);

        //point[0] = pt.X();
        //point[1] = pt.Y();
        //point[2] = pt.Z();
    }

    QList<int> WidgetOCCEvent::getFaces(QList<GraphData::PickedData*> data, bool isOk)
    {
        QList<int> ids = {};
        for (auto d : data) {
            if(d == nullptr)continue;
            ids.append(d->getPickedIds());
        }

        return ids;
    }
}
