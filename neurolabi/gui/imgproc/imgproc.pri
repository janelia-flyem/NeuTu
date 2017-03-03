#Project module for image processing
HEADERS += $${PWD}/zstackprocessor.h \
   $${PWD}/zstackwatershed.h \
    $$PWD/zstackmultiscalewatershed.h \
    $$PWD/zstackgradient.h

SOURCES += $${PWD}/zstackprocessor.cpp \
   $${PWD}/zstackwatershed.cpp \
    $$PWD/zstackmultiscalewatershed.cpp \
    $$PWD/zstackgradient.cpp
