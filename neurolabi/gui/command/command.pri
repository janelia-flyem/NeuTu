HEADERS += \
    $$PWD/zbodyprocesscommand.h \
    $$PWD/zneurontracecommand.h \
    $$PWD/zsyncskeletoncommand.h \
    $$PWD/ztransferskeletoncommand.h \
    $$PWD/zuploadroicommand.h \
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
    $$PWD/zbodyprocesscommand.cpp \
    $$PWD/zneurontracecommand.cpp \
    $$PWD/zsyncskeletoncommand.cpp \
    $$PWD/ztransferskeletoncommand.cpp \
    $$PWD/zuploadroicommand.cpp \
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
