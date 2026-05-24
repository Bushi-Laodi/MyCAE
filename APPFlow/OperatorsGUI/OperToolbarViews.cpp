/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperToolbarViews.h"
#

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGraphWidget.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Component/FITKRenderWindowVTK/FITKGraph3DWindowVTK.h"
#include <QMetaEnum>
#include <QDialog>
#include <QMdiSubWindow>

namespace GUIOper
{
    //三维数组赋值
    void SetValue(double(&array)[3], double v1, double v2, double v3)
    {
        array[0] = v1;
        array[1] = v2;
        array[2] = v3;
    }

    OperToolbarViews::ViewDir String2Enum(const QString string)
    {
        //字符串转枚举
        QMetaEnum metaEnm = QMetaEnum::fromType< OperToolbarViews::ViewDir >();
        return (OperToolbarViews::ViewDir)metaEnm.keyToValue(string.toStdString().data());
    }
    QString Enum2String(OperToolbarViews::ViewDir v)
    {
        //枚举转字符串
        QMetaEnum metaEnm = QMetaEnum::fromType< OperToolbarViews::ViewDir >();
        return metaEnm.valueToKey(v);
    }

    OperToolbarViews::OperToolbarViews()
    {
        init();
    }

    void OperToolbarViews::init()
    {
        auto setting = FITKAPP->getAppSettings();
        if (setting == nullptr)return;

        CameraPara* camPara = new CameraPara;

        camPara->_type = Enum2String(Front);
        QString posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        QString focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        QString vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        QString posValue = setting->getValue<QString>(posKey);
        QString focValue = setting->getValue<QString>(focKey);
        QString vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            SetValue(camPara->_position, 0, 0, 1);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 1, 0);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Back);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Back);
            SetValue(camPara->_position, 0, 0, -1);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 1, 0);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Top);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Top);
            SetValue(camPara->_position, 0, 1, 0);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 0, -1);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Bottom);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Bottom);
            SetValue(camPara->_position, 0, -1, 0);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 0, 1);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Left);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Left);
            SetValue(camPara->_position, -1, 0, 0);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 1, 0);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Right);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Right);
            SetValue(camPara->_position, 1, 0, 0);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 1, 0);
            saveValue(camPara);
        }

        camPara->_type = Enum2String(Isometric);
        posKey = QString("ViewSetting/%1_Position").arg(camPara->_type);
        focKey = QString("ViewSetting/%1_Focal").arg(camPara->_type);
        vieKey = QString("ViewSetting/%1_ViewUp").arg(camPara->_type);
        posValue = setting->getValue<QString>(posKey);
        focValue = setting->getValue<QString>(focKey);
        vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")
        {
            camPara->_type = Enum2String(Isometric);
            SetValue(camPara->_position, 1, 1, 1);
            SetValue(camPara->_focalPoint, 0, 0, 0);
            SetValue(camPara->_viewUp, 0, 1, 0);
            saveValue(camPara);
        }

        delete camPara;
    }

    //点击按钮后执行的GUI操作
    bool OperToolbarViews::execGUI()
    {
        //视角切换由三维渲染组件执行，操作器中不再重复执行
        return false;

        //if (FITKAPP == nullptr)return false;

        //if (_emitter == nullptr) return false;
        //auto setting = FITKAPP->getAppSettings();
        //if (setting == nullptr)return false;

        //GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());

        //QWidget* viewPort = 

        ////声明一个视角
        //ViewDir dir;

        ////获取触发器的objectName
        //const QString objName = _emitter->objectName().toLower();

        ////自适应大小
        //if (objName == "actionviewfit")
        //{
        //    Core::FITKAbstractGraph3DWidget* graph3D = dynamic_cast<Core::FITKAbstractGraph3DWidget*>(viewPort);
        //    if (graph3D == nullptr);
        //    graph3D->fitView();
        //    return true;
        //}

        ////如果是切换front视角
        //else if (objName == "actionviewfront")
        //    //设置视角为front
        //    dir = ViewDir::Front;
        //else if (objName == "actionviewback")
        //    dir = ViewDir::Back;
        //else if (objName == "actionviewtop")
        //    dir = ViewDir::Top;
        //else if (objName == "actionviewbottom")
        //    dir = ViewDir::Bottom;
        //else if (objName == "actionviewleft")
        //    dir = ViewDir::Left;
        //else if (objName == "actionviewright")
        //    dir = ViewDir::Right;
        //else if (objName == "actionviewiso")
        //    dir = ViewDir::Isometric;

        ////初始化三个double数组
        //double focalPoint[3]{ 0 }, viewUp[3]{ 0 }, position[3]{ 0 };

        ////根据枚举获取相机的参数，并传给上面的数组
        //bool   ok = getCameraPara(dir, focalPoint, viewUp, position);
        //if (!ok) return false;

        ////将视角数组设置给相机设置给
        //Comp::FITKGraph3DWindowVTK* graph3D = dynamic_cast<Comp::FITKGraph3DWindowVTK*>(viewPort);
        //if (graph3D) {
        //    graph3D->setView(focalPoint, viewUp, position);
        //}

        return true;
    }

    bool OperToolbarViews::getCameraPara(ViewDir d, double * focalPoint, double * viewupDir, double * cPos)
    {
        auto setting = FITKAPP->getAppSettings();
        if (setting == nullptr)return false;

        QString type = Enum2String(d);
        QString posKey = QString("ViewSetting/%1_Position").arg(type);
        QString focKey = QString("ViewSetting/%1_Focal").arg(type);
        QString vieKey = QString("ViewSetting/%1_ViewUp").arg(type);

        auto String2Array = [&](double* arr, QString s) {
            QStringList sl = s.split(",");
            if (sl.size() != 3) return;
            for (int i = 0; i < 3; i++) arr[i] = sl[i].toDouble();
        };

        QString posValue = setting->getValue<QString>(posKey);
        QString focValue = setting->getValue<QString>(focKey);
        QString vieValue = setting->getValue<QString>(vieKey);
        if (posValue == "" || focValue == "" || vieValue == "")return false;
        String2Array(cPos, posValue);
        String2Array(focalPoint, focValue);
        String2Array(viewupDir, vieValue);
        return true;
    }

    bool OperToolbarViews::saveValue(CameraPara* camPara)
    {
        auto setting = FITKAPP->getAppSettings();
        if (setting == nullptr)return false;

        QString type = camPara->_type;
        QString posKey = QString("ViewSetting/%1_Position").arg(type);
        QString focKey = QString("ViewSetting/%1_Focal").arg(type);
        QString vieKey = QString("ViewSetting/%1_ViewUp").arg(type);

        QString posValue = QString("%1,%2,%3").arg(camPara->_position[0]).arg(camPara->_position[1]).arg(camPara->_position[2]);
        QString focValue = QString("%1,%2,%3").arg(camPara->_focalPoint[0]).arg(camPara->_focalPoint[1]).arg(camPara->_focalPoint[2]);
        QString vieValue = QString("%1,%2,%3").arg(camPara->_viewUp[0]).arg(camPara->_viewUp[1]).arg(camPara->_viewUp[2]);

        setting->setValue(posKey, posValue);
        setting->setValue(focKey, focValue);
        setting->setValue(vieKey, vieValue);
        return true;
    }

}  // namespace GUIOper
