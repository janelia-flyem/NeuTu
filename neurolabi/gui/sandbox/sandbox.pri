HEADERS += $$PWD/zsandbox.h \
    $$PWD/zexamplemodule.h \
    $$PWD/zsandboxmodule.h \
    $$PWD/zsandboxproject.h \
    $$PWD/zaboutmodule.h \
    $$PWD/zrgb2graymodule.h \
    $$PWD/ztracemodule.h \
    $$PWD/qcustomplot.h \
    $$PWD/zimageinfomodule.h \
    $$PWD/zgradientmagnitudemodule.h \
    $$PWD/zmultiscalewatershedmodule.h \
    $$PWD/zshowsegresult.h \
    $$PWD/zbrowseropener.h \
    $$PWD/zffnskeleton.h \
    $$PWD/zmultiscalesegmentationmanagement.h \
    $$PWD/segment/zsegmentationencoder.h \
    $$PWD/segment/zsegmentationtree.h \
    $$PWD/segment/zsegmentationnode.h \
    $$PWD/segment/zsegmentationnodewrapper.h \
    $$PWD/segment/zmstcontainer.h

SOURCES += $$PWD/zsandbox.cpp \
    $$PWD/zexamplemodule.cpp \
    $$PWD/zsandboxmodule.cpp \
    $$PWD/zsandboxproject.cpp \
    $$PWD/zaboutmodule.cpp \
    $$PWD/zrgb2graymodule.cpp \
    $$PWD/ztracemodule.cpp \
    $$PWD/qcustomplot.cpp \
    $$PWD/zimageinfomodule.cpp \
    $$PWD/zgradientmagnitudemodule.cpp \
    $$PWD/zmultiscalewatershedmodule.cpp \
    $$PWD/zshowsegresult.cpp \
    $$PWD/zbrowseropener.cpp \
    $$PWD/zffnskeleton.cpp \
    $$PWD/zmultiscalesegmentationmanagement.cpp \
    $$PWD/segment/zsegmentationencoder.cpp \
    $$PWD/segment/zsegmentationtree.cpp \
    $$PWD/segment/zsegmentationnode.cpp \
    $$PWD/segment/zsegmentationnodewrapper.cpp \
    $$PWD/segment/zmstcontainer.cpp


CONFIG(surfrecon) {
  HEADERS += \
    $$PWD/zsurfreconmodule.h\
    $$PWD/surfrecon.h \
    $$PWD/zsurfreconmodule.h

  SOURCES += \
    $$PWD/zsurfreconmodule.cpp\
    $$PWD/zsurfreconmodule.cpp
}
