include(tests/tests.pri)

HEADERS += \
    $$PWD/defs.h \
    $$PWD/zlog.h \
    $$PWD/neuopentracing.h \
    $$PWD/utilities.h \
    $$PWD/zbenchtimer.h \
    $$PWD/zqslog.h \
    $$PWD/zloggable.h

SOURCES += \
    $$PWD/zlog.cpp \
    $$PWD/neuopentracing.cpp \
    $$PWD/zbenchtimer.cpp \
    $$PWD/utilities.cpp \
    $$PWD/zloggable.cpp
