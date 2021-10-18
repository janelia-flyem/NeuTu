include($$PWD/tests/tests.pri)

SOURCES += \
  $$PWD/displayconfig.cpp \
  $$PWD/zmodelviewtransform.cpp \
  $$PWD/zsliceviewtransform.cpp \
  $$PWD/zstackobjecthandle.cpp \
  $$PWD/zviewplanetransform.cpp \
  $${PWD}/utilities.cpp \
  $${PWD}/zstackobjectconfig.cpp \
  $${PWD}/zstackobjecthelper.cpp

HEADERS += \
  $$PWD/defs.h \
  $$PWD/displayconfig.h \
  $$PWD/zmodelviewtransform.h \
  $$PWD/zsliceviewtransform.h \
  $$PWD/zstackobjecthandle.h \
  $$PWD/zviewplanetransform.h \
  $${PWD}/utilities.h \
  $${PWD}/zstackobjectconfig.h \
  $${PWD}/zstackobjecthelper.h
