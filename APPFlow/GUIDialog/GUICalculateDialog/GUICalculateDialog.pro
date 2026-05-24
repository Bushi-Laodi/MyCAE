TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUICalculateDialog
QT          +=  core widgets gui
DEFINES     +=  GUICalculateDialog_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./GUICalculateDialog.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../../output/bin
    Release:MOC_DIR         = ../../generate/GUICalculateDialog/release/moc
    Release:RCC_DIR         = ../../generate/GUICalculateDialog/release/rcc
    Release:UI_DIR          = ../../generate/GUICalculateDialog/release/qui
    Release:OBJECTS_DIR     = ../../generate/GUICalculateDialog/release/obj
    Release:LIBS +=  \
        -L../../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceMesh \
        -lFITKOFDictWriter \
        -lFITKInterfaceGeometry \
        -lFITKOFDriver \
        -lFITKEasyParam \
   


    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../../output/bin_d
    Debug:MOC_DIR         = ../../generate/GUICalculateDialog/debug/moc
    Debug:RCC_DIR         = ../../generate/GUICalculateDialog/debug/rcc
    Debug:UI_DIR          = ../../generate/GUICalculateDialog/debug/qui
    Debug:OBJECTS_DIR     = ../../generate/GUICalculateDialog/debug/obj
    Debug:LIBS +=  \
        -L../../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceMesh \
        -lFITKOFDictWriter \
        -lFITKInterfaceGeometry \
        -lFITKOFDriver \
        -lFITKEasyParam \
     


    message("Windows GUICalculateDialog generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../../output/bin
    MOC_DIR         = ../../generate/GUICalculateDialog/release/moc
    UI_DIR          = ../../generate/GUICalculateDialog/release/qui
    RCC_DIR         = ../../generate/GUICalculateDialog/release/rcc
    OBJECTS_DIR     = ../../generate/GUICalculateDialog/release/obj
    LIBS += \
        -L../../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lGUIWidget \
        -lGUIFrame \
        -lFITKWidget \
        -lFITKInterfaceFlowOF \
        -lFITKInterfaceMesh \
        -lFITKOFDictWriter \
        -lFITKInterfaceGeometry \
        -lFITKOFDriver \
        -lFITKEasyParam \
       

    message("Linux GUICalculateDialog generated")
}
