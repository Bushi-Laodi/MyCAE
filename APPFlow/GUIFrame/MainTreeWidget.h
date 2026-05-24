/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _MainTreeWidget_H
#define _MainTreeWidget_H

#include "GUIFrameAPI.h"
#include "PanelWidgetBase.h"

namespace GUI
{
    class MainWindow;
    class TreeWidget;

    enum class MainTreeEnum {
        MainTree_None = 0,
        MainTree_Geomety,
        MainTree_GeometyBoxItem,
        MainTree_GeometyCylinderItem,
        MainTree_GeometySphereItem,
        MainTree_GeometyBoolOrImportItem,
        
        MainTree_Mesh,
        MainTree_MeshGeometry,
        MainTree_MeshBase,
        MainTree_MeshLocal,
        MainTree_MeshLocalItem,
        MainTree_MeshPoint,
        MainTree_MeshBoundary,
        MainTree_MeshBoundaryItem,

        MainTree_Setup,
        MainTree_SetupRegionMesh,
        MainTree_SetupRadiation,
        MainTree_SetupTurbulence,
        MainTree_SetupThermo,
        MainTree_SetupTransportProperties,
        MainTree_SetupDiscretization,
        MainTree_SetupSolution,
        MainTree_SetupPassiveScalars,
        MainTree_SetupOperatingConditions,
        MainTree_SetupCellZones,
        MainTree_SetupBoundaryConditions,
        MainTree_SetupBoundaryConditionsItem,
        MainTree_SetupInitialConditions,
        MainTree_SetupMonitors,

        MainTree_Run,
        MainTree_Post,
    };

    class GUIFRAMEAPI MainTreeWidget : public PanelWidgetBase
    {
        Q_OBJECT;
    public:
        MainTreeWidget(MainWindow* parent);
        ~MainTreeWidget();

        void init();

        TreeWidget* getTreeWidget();

    private:
        TreeWidget* _treeWidget = nullptr;
    };
}

#endif