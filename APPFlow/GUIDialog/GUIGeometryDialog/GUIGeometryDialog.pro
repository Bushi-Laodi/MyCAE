TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIGeometryDialog
QT          +=  core widgets gui
DEFINES     +=  GUIGeometryDialog_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./GUIGeometryDialog.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../../output/bin
    Release:MOC_DIR         = ../../generate/GUIGeometryDialog/release/moc
    Release:RCC_DIR         = ../../generate/GUIGeometryDialog/release/rcc
    Release:UI_DIR          = ../../generate/GUIGeometryDialog/release/qui
    Release:OBJECTS_DIR     = ../../generate/GUIGeometryDialog/release/obj
    Release:LIBS +=  \
        -L../../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceMeshGen \
   


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../../output/bin_d
    Debug:MOC_DIR         = ../../generate/GUIGeometryDialog/debug/moc
    Debug:RCC_DIR         = ../../generate/GUIGeometryDialog/debug/rcc
    Debug:UI_DIR          = ../../generate/GUIGeometryDialog/debug/qui
    Debug:OBJECTS_DIR     = ../../generate/GUIGeometryDialog/debug/obj
    Debug:LIBS +=  \
        -L../../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceMeshGen \
     


    message("Windows GUIGeometryDialog generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../../output/bin
    MOC_DIR         = ../../generate/GUIGeometryDialog/release/moc
    UI_DIR          = ../../generate/GUIGeometryDialog/release/qui
    RCC_DIR         = ../../generate/GUIGeometryDialog/release/rcc
    OBJECTS_DIR     = ../../generate/GUIGeometryDialog/release/obj
    LIBS += \
        -L../../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceGeometry \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceModel \
        -lFITKInterfaceMeshGen \
       

    message("Linux GUIGeometryDialog generated")
}
