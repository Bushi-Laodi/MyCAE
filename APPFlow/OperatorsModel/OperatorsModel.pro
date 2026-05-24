TEMPLATE    =  lib
CONFIG     +=  c++11
CONFIG     +=  qt
TARGET      =  OperatorsModel
QT         +=  core widgets gui
DEFINES    +=  OperatorsModel_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./OperatorsModel.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/OperatorsModel/release/moc
    Release:RCC_DIR         = ../generate/OperatorsModel/release/rcc
    Release:UI_DIR          = ../generate/OperatorsModel/release/qui
    Release:OBJECTS_DIR     = ../generate/OperatorsModel/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKOFDictWriter \
        -lGUIFrame \
        -lGUIWidget \
        -lOperatorsInterface \
        -lGUIGeometryDialog \
        -lGUIMeshDialog \
        -lGUICalculateDialog \
        -lFITKOFDriver \
        -lFITKInterfaceIO \
        -lFITKInterfaceMesh \
        -lFITKOFMeshIO \
        -lFITKInterfaceMeshGen \
        -lFITKGeoCompOCC \
        -lFITKInterfaceModel \
        -lFITKInterfaceGeometry \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKFlowOFIOHDF5 \

    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/OperatorsModel/debug/moc
    Debug:RCC_DIR         = ../generate/OperatorsModel/debug/rcc
    Debug:UI_DIR          = ../generate/OperatorsModel/debug/qui
    Debug:OBJECTS_DIR     = ../generate/OperatorsModel/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKOFDictWriter \
        -lGUIFrame \
        -lGUIWidget \
        -lOperatorsInterface \
        -lGUIGeometryDialog \
        -lGUIMeshDialog \
        -lGUICalculateDialog \
        -lFITKOFDriver \
        -lFITKInterfaceIO \
        -lFITKInterfaceMesh \
        -lFITKOFMeshIO \
        -lFITKInterfaceMeshGen\
        -lFITKGeoCompOCC \
        -lFITKInterfaceModel \
        -lFITKInterfaceGeometry \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKFlowOFIOHDF5 \

    message("Windows OperatorsModel generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/OperatorsModel/release/moc
    UI_DIR          = ../generate/OperatorsModel/release/qui
    RCC_DIR         = ../generate/OperatorsModel/release/rcc
    OBJECTS_DIR     = ../generate/OperatorsModel/release/obj
    LIBS += \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKOFDictWriter \
        -lGUIFrame \
        -lGUIWidget \
        -lOperatorsInterface \
        -lGUIGeometryDialog \
        -lGUIMeshDialog \
        -lGUICalculateDialog \
        -lFITKOFDriver \
        -lFITKInterfaceIO \
        -lFITKInterfaceMesh \
        -lFITKOFMeshIO \
        -lFITKInterfaceMeshGen \
        -lFITKGeoCompOCC \
        -lFITKInterfaceModel \
        -lFITKInterfaceGeometry \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKFlowOFIOHDF5 \

    message("Linux OperatorsModel generated")
}

