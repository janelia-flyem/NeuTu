HEADERS += sandbox/zsandbox.h \
    sandbox/zexamplemodule.h \
    sandbox/zsandboxmodule.h \
    sandbox/zsandboxproject.h \
    sandbox/zaboutmodule.h \
    sandbox/zrgb2graymodule.h \
    $$PWD/ztracemodule.h \
    $$PWD/qcustomplot.h \
    $$PWD/zimageinfomodule.h \
    $$PWD/zgradientmagnitudemodule.h \
    $$PWD/zmultiscalewatershedmodule.h

SOURCES += sandbox/zsandbox.cpp \
    sandbox/zexamplemodule.cpp \
    sandbox/zsandboxmodule.cpp \
    sandbox/zsandboxproject.cpp \
    sandbox/zaboutmodule.cpp \
    sandbox/zrgb2graymodule.cpp \
    $$PWD/ztracemodule.cpp \
    $$PWD/qcustomplot.cpp \
    $$PWD/zimageinfomodule.cpp \
    $$PWD/zgradientmagnitudemodule.cpp \
    $$PWD/zmultiscalewatershedmodule.cpp

contains(CONFIG, surfrecon) {
  HEADERS += \
    $$PWD/surfrecon.h \
    $$PWD/zsurfreconmodule.h

  SOURCES += \
    $$PWD/zsurfreconmodule.cpp
}
