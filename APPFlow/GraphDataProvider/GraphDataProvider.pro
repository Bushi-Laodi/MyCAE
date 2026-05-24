TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GraphDataProvider
QT          +=  core widgets gui
DEFINES     +=  GRAPHDATAPROVIDER_API

win32{
    QMAKE_CXXFLAGS += /MP
}

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./vtk.pri)
include(./GraphDataProvider.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/GraphDataProvider/release/moc
    Release:RCC_DIR         = ../generate/GraphDataProvider/release/rcc
    Release:UI_DIR          = ../generate/GraphDataProvider/release/qui
    Release:OBJECTS_DIR     = ../generate/GraphDataProvider/release/obj
    Release:LIBS +=  \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceMesh \
        -lFITKInterfaceMeshGen \
        -lFITKAdaptor \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKRenderWindowVTK \
        -lFITKVTKAlgorithm \
        -lGUIFrame

    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/GraphDataProvider/debug/moc
    Debug:RCC_DIR         = ../generate/GraphDataProvider/debug/rcc
    Debug:UI_DIR          = ../generate/GraphDataProvider/debug/qui
    Debug:OBJECTS_DIR     = ../generate/GraphDataProvider/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceMesh \
        -lFITKInterfaceMeshGen \
        -lFITKAdaptor \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKRenderWindowVTK \
        -lFITKVTKAlgorithm \
        -lGUIFrame

    message("Windows GraphDataProvider generated")
}

unix{

    INCLUDEPATH    +=   ./  \
                        ../ \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/GraphDataProvider/release/moc
    UI_DIR          = ../generate/GraphDataProvider/release/qui
    RCC_DIR         = ../generate/GraphDataProvider/release/rcc
    OBJECTS_DIR     = ../generate/GraphDataProvider/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceMesh \
        -lFITKInterfaceMeshGen \
        -lFITKAdaptor \
        -lFITKFluidVTKGraphAdaptor \
        -lFITKRenderWindowVTK \
        -lFITKVTKAlgorithm \
        -lGUIFrame

    message("Linux GraphDataProvider generated")
}