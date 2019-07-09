HEADERS += \  
    $$PWD/zcommandmodule.h \
    $$PWD/zstackdownsamplecommand.h \
    $$PWD/zbodysplitcommand.h \
    $$PWD/zsurfreconcommand.h \
    $$PWD/zstackdiffcommand.h \
    $$PWD/zmultiscalewatershedcommand.h \
    $$PWD/zbodyexportcommand.h \
    $$PWD/zsparsestackcommandmodule.h \
    $$PWD/zstackfiltercommand.h

SOURCES += \ 
    $$PWD/zcommandmodule.cpp \
    $$PWD/zstackdownsamplecommand.cpp \
    $$PWD/zbodysplitcommand.cpp \
    $$PWD/zsurfreconcommand.cpp \
    $$PWD/zstackdiffcommand.cpp \
    $$PWD/zmultiscalewatershedcommand.cpp \
    $$PWD/zbodyexportcommand.cpp \
    $$PWD/zsparsestackcommandmodule.cpp \
    $$PWD/zstackfiltercommand.cpp

contains(DEFINES, _FLYEM_) {
  message("command.pri")
  HEADERS += \
    $$PWD/zsplittaskuploadcommand.h

  SOURCES += \
    $$PWD/zsplittaskuploadcommand.cpp
}
