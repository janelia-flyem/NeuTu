#Project module for image processing
HEADERS += $${PWD}/zstackprocessor.h \
   $${PWD}/zstackwatershed.h \
    $$PWD/zstackgradient.h \
    $$PWD/zdownsamplefilter.h \
    $$PWD/zstackprinter.h

SOURCES += $${PWD}/zstackprocessor.cpp \
   $${PWD}/zstackwatershed.cpp \
    $$PWD/zstackgradient.cpp \
    $$PWD/zdownsamplefilter.cpp \
    $$PWD/zstackprinter.cpp

contains(DEFINES, _ENABLE_SURFRECON_) {
  HEADERS +=  \
    $$PWD/zsurfrecon.h \
    $$PWD/surfrecon.h

  SOURCES += \
    $$PWD/zsurfrecon.cpp \
    $$PWD/surfrecon.cpp
}
