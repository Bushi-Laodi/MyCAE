TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIWidget
QT          +=  core widgets gui
DEFINES     += GUIWidget_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

#include(./occ.pri)
include(./vtk.pri)
include(./GUIWidget.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \
                       
    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/GUIWidget/release/moc
    Release:RCC_DIR         = ../generate/GUIWidget/release/rcc
    Release:UI_DIR          = ../generate/GUIWidget/release/qui
    Release:OBJECTS_DIR     = ../generate/GUIWidget/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lFITKWidget \
        -lOperatorsInterface \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKGeoCompOCC \
        -lFITKInterfaceMeshGen \
        -lFITKInterfaceMesh \
        
    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/GUIWidget/debug/moc
    Debug:RCC_DIR         = ../generate/GUIWidget/debug/rcc
    Debug:UI_DIR          = ../generate/GUIWidget/debug/qui
    Debug:OBJECTS_DIR     = ../generate/GUIWidget/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lFITKWidget \
        -lOperatorsInterface \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKGeoCompOCC \
        -lFITKInterfaceMeshGen \
        -lFITKInterfaceMesh \

    message("Windows GUIWidget generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/GUIWidget/release/moc
    UI_DIR          = ../generate/GUIWidget/release/qui
    RCC_DIR         = ../generate/GUIWidget/release/rcc
    OBJECTS_DIR     = ../generate/GUIWidget/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKPython \
        -lFITKCore \
        -lFITKWidget \
        -lOperatorsInterface \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKVTKAlgorithm \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKGeoCompOCC \
        -lFITKInterfaceMeshGen \
        -lFITKInterfaceMesh \

    message("Linux GUIWidget generated")
}

