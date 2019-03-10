HEADERS += sandbox/zsandbox.h \
    sandbox/zexamplemodule.h \
    sandbox/zsandboxmodule.h \
    sandbox/zsandboxproject.h \
    sandbox/zaboutmodule.h \
    sandbox/zrgb2graymodule.h \
    sandbox/zsurfreconmodule.h\
    $$PWD/ztracemodule.h \
    $$PWD/qcustomplot.h \
    $$PWD/zimageinfomodule.h \
    $$PWD/zgradientmagnitudemodule.h \
    $$PWD/zmultiscalewatershedmodule.h \
    $$PWD/zshowsegresult.h \
    $$PWD/zbrowseropener.h \
    $$PWD/zffnskeleton.h \
    $$PWD/zmultiscalesegmentationmanagement.h \
    $$PWD/zsegmentationrepresentation.h

SOURCES += sandbox/zsandbox.cpp \
    sandbox/zsurfreconmodule.cpp\
    sandbox/zexamplemodule.cpp \
    sandbox/zsandboxmodule.cpp \
    sandbox/zsandboxproject.cpp \
    sandbox/zaboutmodule.cpp \
    sandbox/zrgb2graymodule.cpp \
    $$PWD/ztracemodule.cpp \
    $$PWD/qcustomplot.cpp \
    $$PWD/zimageinfomodule.cpp \
    $$PWD/zgradientmagnitudemodule.cpp \
    $$PWD/zmultiscalewatershedmodule.cpp \
    $$PWD/zshowsegresult.cpp \
    $$PWD/zbrowseropener.cpp \
    $$PWD/zffnskeleton.cpp \
    $$PWD/zmultiscalesegmentationmanagement.cpp \
    $$PWD/zsegmentationrepresentation.cpp

contains(CONFIG, surfrecon) {
  HEADERS += \
    $$PWD/surfrecon.h \
    $$PWD/zsurfreconmodule.h

  SOURCES += \
    $$PWD/zsurfreconmodule.cpp
}
