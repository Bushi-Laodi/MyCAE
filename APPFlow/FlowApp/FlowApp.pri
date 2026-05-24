HEADERS += \ 
    $$PWD/MainWindowGenerator.h \
    $$PWD/GlobalDataFactory.h \
    $$PWD/ComponentFactory.h \ 
    $$PWD/CommandLineHandler.h \
    $$PWD/PreWindowInitializer.h \
    $$PWD/SignalProcessor.h \
    $$PWD/FlowAPPSettings.h \
    $$PWD/WorkBenchHandler.h \
    $$PWD/SystemChecker.h \

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/MainWindowGenerator.cpp \
    $$PWD/GlobalDataFactory.cpp \
    $$PWD/ComponentFactory.cpp \ 
    $$PWD/CommandLineHandler.cpp \
    $$PWD/PreWindowInitializer.cpp \
    $$PWD/SignalProcessor.cpp \
    $$PWD/FlowAPPSettings.cpp \
    $$PWD/WorkBenchHandler.cpp \
    $$PWD/SystemChecker.cpp \

FORMS += \
#    $$PWD/PluginManageDialog.ui \
 
 
#RESOURCES += ../qrc/qianfan.qrc

#RC_FILE += ../qrc/qianfan.rc
RC_ICONS = icon.ico 