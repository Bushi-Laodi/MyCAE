TEMPLATE    =   lib
CONFIG      +=  c++11
CONFIG      +=  qt
TARGET      =   OperatorsInterface
QT          +=  core widgets gui
DEFINES     += OperatorsInterface_API

unix:!mac{ QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/\'" }

include(./OperatorsInterface.pri)

win32{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Win64/SARibbon/include/SARibbon-2.0.1/  \

    Release:DESTDIR         = ../output/bin
    Release:MOC_DIR         = ../generate/OperatorsInterface/release/moc
    Release:RCC_DIR         = ../generate/OperatorsInterface/release/rcc
    Release:UI_DIR          = ../generate/OperatorsInterface/release/qui
    Release:OBJECTS_DIR     = ../generate/OperatorsInterface/release/obj
    Release:LIBS +=  \
        -L../output/bin  \
        -lFITKAppFramework \
        -lFITKCore \
        
    Debug:CONFIG            +=  console
    Debug:DESTDIR         = ../output/bin_d
    Debug:MOC_DIR         = ../generate/OperatorsInterface/debug/moc
    Debug:RCC_DIR         = ../generate/OperatorsInterface/debug/rcc
    Debug:UI_DIR          = ../generate/OperatorsInterface/debug/qui
    Debug:OBJECTS_DIR     = ../generate/OperatorsInterface/debug/obj
    Debug:LIBS +=  \
        -L../output/bin_d \
        -lFITKAppFramework \
        -lFITKCore \


    message("Windows OperatorsInterface generated")
}

unix{

    INCLUDEPATH    +=  ./   \
                       ../  \
                       ../../ \
                       ../Tools/Linux64/SARibbon/include/  \

    CONFIG          += console
    CONFIG          += plugin
    DESTDIR         = ../output/bin
    MOC_DIR         = ../generate/OperatorsInterface/release/moc
    UI_DIR          = ../generate/OperatorsInterface/release/qui
    RCC_DIR         = ../generate/OperatorsInterface/release/rcc
    OBJECTS_DIR     = ../generate/OperatorsInterface/release/obj
    LIBS += \
        -L../output/bin \
        -lFITKAppFramework \
        -lFITKCore \
   

    message("Linux OperatorsInterface generated")
}

