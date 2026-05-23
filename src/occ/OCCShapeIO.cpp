#include "OCCShapeIO.h"

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>

#include <QFileInfo>
#include <QDir>

namespace
{
bool ensureParentDirectory(const QString &filePath, QString *errorMessage)
{
    const QFileInfo fileInfo(filePath);
    const QString parentPath = fileInfo.absolutePath();
    if (QDir().mkpath(parentPath)) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = "Failed to create parent directory: " + parentPath;
    }
    return false;
}
}

bool OCCShapeIO::saveBREP(const TopoDS_Shape &shape, const QString &filePath, QString *errorMessage) const
{
    if (shape.IsNull()) {
        if (errorMessage) {
            *errorMessage = "Cannot save a null TopoDS_Shape as BREP.";
        }
        return false;
    }

    if (!ensureParentDirectory(filePath, errorMessage)) {
        return false;
    }

    try {
        if (!BRepTools::Write(shape, filePath.toLocal8Bit().constData())) {
            if (errorMessage) {
                *errorMessage = "BRepTools::Write returned false.";
            }
            return false;
        }
        return true;
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = QString("BRepTools::Write failed: %1").arg(failure.GetMessageString());
        }
        return false;
    }
}

bool OCCShapeIO::loadBREP(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage) const
{
    if (!QFileInfo::exists(filePath)) {
        if (errorMessage) {
            *errorMessage = "BREP file does not exist: " + filePath;
        }
        return false;
    }

    try {
        BRep_Builder builder;
        TopoDS_Shape loadedShape;
        if (!BRepTools::Read(loadedShape, filePath.toLocal8Bit().constData(), builder)) {
            if (errorMessage) {
                *errorMessage = "BRepTools::Read returned false.";
            }
            return false;
        }

        if (loadedShape.IsNull()) {
            if (errorMessage) {
                *errorMessage = "BRepTools::Read produced a null TopoDS_Shape.";
            }
            return false;
        }

        shape = loadedShape;
        return true;
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = QString("BRepTools::Read failed: %1").arg(failure.GetMessageString());
        }
        return false;
    }
}

bool OCCShapeIO::saveSTEP(const TopoDS_Shape &shape, const QString &filePath, QString *errorMessage) const
{
    if (shape.IsNull()) {
        if (errorMessage) {
            *errorMessage = "Cannot save a null TopoDS_Shape as STEP.";
        }
        return false;
    }

    if (!ensureParentDirectory(filePath, errorMessage)) {
        return false;
    }

    try {
        STEPControl_Writer writer;
        const IFSelect_ReturnStatus transferStatus = writer.Transfer(shape, STEPControl_AsIs);
        if (transferStatus != IFSelect_RetDone) {
            if (errorMessage) {
                *errorMessage = QString("STEP transfer failed with status %1.").arg(static_cast<int>(transferStatus));
            }
            return false;
        }

        const IFSelect_ReturnStatus writeStatus = writer.Write(filePath.toLocal8Bit().constData());
        if (writeStatus != IFSelect_RetDone) {
            if (errorMessage) {
                *errorMessage = QString("STEP write failed with status %1.").arg(static_cast<int>(writeStatus));
            }
            return false;
        }
        return true;
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = QString("STEP write failed: %1").arg(failure.GetMessageString());
        }
        return false;
    }
}

bool OCCShapeIO::loadSTEP(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage) const
{
    if (!QFileInfo::exists(filePath)) {
        if (errorMessage) {
            *errorMessage = "STEP file does not exist: " + filePath;
        }
        return false;
    }

    try {
        STEPControl_Reader reader;
        const IFSelect_ReturnStatus readStatus = reader.ReadFile(filePath.toLocal8Bit().constData());
        if (readStatus != IFSelect_RetDone) {
            if (errorMessage) {
                *errorMessage = QString("STEP read failed with status %1.").arg(static_cast<int>(readStatus));
            }
            return false;
        }

        const int transferredRoots = reader.TransferRoots();
        if (transferredRoots <= 0) {
            if (errorMessage) {
                *errorMessage = "STEP reader did not transfer any roots.";
            }
            return false;
        }

        TopoDS_Shape loadedShape = reader.OneShape();
        if (loadedShape.IsNull()) {
            if (errorMessage) {
                *errorMessage = "STEP reader produced a null TopoDS_Shape.";
            }
            return false;
        }

        shape = loadedShape;
        return true;
    } catch (const Standard_Failure &failure) {
        if (errorMessage) {
            *errorMessage = QString("STEP read failed: %1").arg(failure.GetMessageString());
        }
        return false;
    }
}
