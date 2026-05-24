TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIMeshDialog
QT          +=  core widgets gui
DEFINES     +=  GUIMeshDialog_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./GUIMeshDialog.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../../output/bin
    Release:MOC_DIR         = ../../generate/GUIMeshDialog/release/moc
    Release:RCC_DIR         = ../../generate/GUIMeshDialog/release/rcc
    Release:UI_DIR          = ../../generate/GUIMeshDialog/release/qui
    Release:OBJECTS_DIR     = ../../generate/GUIMeshDialog/release/obj
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
        -lFITKInterfaceMesh \


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../../output/bin_d
    Debug:MOC_DIR         = ../../generate/GUIMeshDialog/debug/moc
    Debug:RCC_DIR         = ../../generate/GUIMeshDialog/debug/rcc
    Debug:UI_DIR          = ../../generate/GUIMeshDialog/debug/qui
    Debug:OBJECTS_DIR     = ../../generate/GUIMeshDialog/debug/obj
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
        -lFITKInterfaceMesh \
     


    message("Windows GUIMeshDialog generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../../output/bin
    MOC_DIR         = ../../generate/GUIMeshDialog/release/moc
    UI_DIR          = ../../generate/GUIMeshDialog/release/qui
    RCC_DIR         = ../../generate/GUIMeshDialog/release/rcc
    OBJECTS_DIR     = ../../generate/GUIMeshDialog/release/obj
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
        -lFITKInterfaceMesh \
       

    message("Linux GUIMeshDialog generated")
}
