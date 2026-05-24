/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "GeometryWidgetBase.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

namespace GUI
{
    GeometryWidgetBase::GeometryWidgetBase(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        Core::FITKWidget(parent), _obj(obj), _oper(oper)
    {
        _mainWin = FITKAPP->getGlobalData()->getMainWindowT<MainWindow>();
    }

    GeometryWidgetBase::~GeometryWidgetBase()
    {

    }

    QList<int> GeometryWidgetBase::getDefaultFaceGroup(Interface::FITKAbsGeoCommand* geoObj)
    {
        QList<int> defaultFaceIDs = {};
        if (geoObj == nullptr)return defaultFaceIDs;
        Interface::FITKAbsGeoShapeAgent* geoShapeAgent = geoObj->getShapeAgent();
        if (geoShapeAgent == nullptr)return defaultFaceIDs;
        Interface::FITKVirtualTopoManager* geoTopoManager = geoShapeAgent->getVirtualTopoManager();
        if (geoTopoManager == nullptr)return defaultFaceIDs;
        Interface::FITKShapeVirtualTopoManager* shapTopeManager = geoTopoManager->getShapeVirtualTopoManager(Interface::FITKGeoEnum::VTopoShapeType::VSFace);
        if (shapTopeManager == nullptr)return defaultFaceIDs;

        for (int i = 0; i < shapTopeManager->getDataCount(); i++) {
            Interface::FITKAbsVirtualTopo* faceTopo = shapTopeManager->getDataByIndex(i);
            if (faceTopo == nullptr)continue;
            defaultFaceIDs.append(faceTopo->getDataObjectID());
        }
        return defaultFaceIDs;
    }

    void GeometryWidgetBase::createDefaultFaceGroup(Interface::FITKAbsGeoCommand* geoObj)
    {
        if (geoObj == nullptr)return;
        Interface::FITKAbsGeoShapeAgent* geoShapeAgent = geoObj->getShapeAgent();
        if (geoShapeAgent == nullptr)return;
        Interface::FITKGeoComponentManager* commanger = geoShapeAgent->getGeoComponentManager();
        if (commanger == nullptr)return;

        QList<int> defaultFaceIDs = getDefaultFaceGroup(geoObj);
        //创建面组对象
        Interface::FITKGeoComponent* geoCom = new Interface::FITKGeoComponent(Interface::FITKModelEnum::FITKModelSetType::FMSSurface);
        geoCom->setDataObjectName("Default");
        geoCom->setMember(defaultFaceIDs);
        commanger->appendDataObj(geoCom);
    }

    Interface::FITKAbsGeoCommand * GeometryWidgetBase::getCurrentGeoCommand()
    {
        return _obj;
    }

    void GeometryWidgetBase::showEvent(QShowEvent * event)
    {
        transparency();
    }

    void GeometryWidgetBase::closeEvent(QCloseEvent * event)
    {
        transparency(false, false);
    }

    void GeometryWidgetBase::transparency(bool geo, bool mesh)
    {
        //EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        //if (graphOper == nullptr)return;
        //graphOper->setEnableModelTransparent(geo);
        //graphOper->setEnableMeshTransparent(mesh);
        //graphOper->reRender();
    }

    void GeometryWidgetBase::createMeshSizeGeo()
    {
        //auto meshSizeGen = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
        //auto meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        //if (meshSizeGen&&meshSizeManager) {
        //    auto meshSizeGeo = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>
        //        (meshSizeGen->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom));
        //    if (meshSizeGeo) {
        //        meshSizeGeo->setGeomID(_obj->getDataObjectID());
        //        meshSizeManager->appendDataObj(meshSizeGeo);
        //    }
        //}
    }
}


