#Project module for image processing
HEADERS += $${PWD}/zstackprocessor.h \
  $$PWD/zfilestacksource.h \
  $$PWD/zstacksource.h \
   $${PWD}/zstackwatershed.h \
    $$PWD/zstackgradient.h \
    $$PWD/zdownsamplefilter.h \
    $$PWD/zstackprinter.h \
    $$PWD/zwatershedmst.h

SOURCES += $${PWD}/zstackprocessor.cpp \
  $$PWD/zfilestacksource.cpp \
  $$PWD/zstacksource.cpp \
   $${PWD}/zstackwatershed.cpp \
    $$PWD/zstackgradient.cpp \
    $$PWD/zdownsamplefilter.cpp \
    $$PWD/zstackprinter.cpp \
    $$PWD/zwatershedmst.cpp

contains(DEFINES, _ENABLE_SURFRECON_) {
  HEADERS +=  \
    $$PWD/zsurfrecon.h \
    $$PWD/surfrecon.h

  SOURCES += \
    $$PWD/zsurfrecon.cpp \
    $$PWD/surfrecon.cpp
}
