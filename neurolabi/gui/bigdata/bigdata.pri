include($$PWD/tests/tests.pri)

HEADERS += $$PWD/zdvidblockgrid.h \
    $$PWD/zchunkdatasource.hpp \
    $$PWD/zintpointannotationblockgrid.hpp \
    $$PWD/zintpointannotationchunk.h \
    $$PWD/zintpointannotationsource.hpp \
    $$PWD/zitemchunk.hpp \
    $$PWD/zmockitemchunk.h \
    $$PWD/zstackblockgrid.h \
    $$PWD/zblockgrid.h \
    $$PWD/zblockgridfactory.h

SOURCES += $$PWD/zdvidblockgrid.cpp \
   $$PWD/zintpointannotationsource.cpp \
   $$PWD/zintpointannotationchunk.cpp \
   $$PWD/zitemchunk.cpp \
   $$PWD/zmockitemchunk.cpp \
   $$PWD/zstackblockgrid.cpp \
   $$PWD/zblockgrid.cpp
