TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   GUIFrame
QT          +=  core widgets gui
DEFINES     += GUIFrame_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./GUIFrame.pri)
#include(./vtk.pri)
 

win32{

    INCLUDEPATH    +=  ./   \
                            ../  \
                             ../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/GUIFrame/release/moc
    Release:RCC_DIR         = ../generate/GUIFrame/release/rcc
    Release:UI_DIR          = ../generate/GUIFrame/release/qui
    Release:OBJECTS_DIR     = ../generate/GUIFrame/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKCompMessageWidget \
        -lFITKWidget \
        -lGUIWidget \
        -lFITKRenderWindowVTK \



    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/GUIFrame/debug/moc
    Debug:RCC_DIR         = ../generate/GUIFrame/debug/rcc
    Debug:UI_DIR          = ../generate/GUIFrame/debug/qui
    Debug:OBJECTS_DIR     = ../generate/GUIFrame/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKCompMessageWidget \
        -lFITKWidget \
        -lGUIWidget \
        -lFITKRenderWindowVTK \

Debug:LIBS +=  -L$$PWD/../Tools/Win64/SARibbon/libd/  \                                  
               -lSARibbonBard  \

Release:LIBS +=  -L$$PWD/../Tools/Win64/SARibbon/lib/  \
               -lSARibbonBar  \

    message("Windows GUIFrame generated")
}

unix{
    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/GUIFrame/release/moc
    UI_DIR          = ../generate/GUIFrame/release/qui
    RCC_DIR         = ../generate/GUIFrame/release/rcc
    OBJECTS_DIR     = ../generate/GUIFrame/release/obj
    INCLUDEPATH    += ./  \
                      ../ \
                      ../Tools/Linux64/SARibbon/include/  \

    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
        -lFITKPython \
        -lFITKCompMessageWidget \
        -lFITKWidget \
        -lGUIWidget \
        -lFITKRenderWindowVTK \
        -L$$PWD/../Tools/Linux64/SARibbon/lib/  \
        -lSARibbonBar \

    message("Linux GUIFrame generated")
}

RESOURCES += \
    ../Resources/Resources.qrc

