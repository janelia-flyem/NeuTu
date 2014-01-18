DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext

unix {
    INCLUDEPATH += $${EXTLIB_DIR}/xml/include/libxml2 \
        $${EXTLIB_DIR}/fftw3/include \
        $${EXTLIB_DIR}/png/include \
        $${EXTLIB_DIR}/jansson/include
}

win32 {
    INCLUDEPATH += C:/Mingw/include \
        C:/Mingw/include/libxml2 \
        C:/Qt/2010.05/mingw/include/libxml2

    LIBS += -LC:/Mingw/lib \
        -lfftw3 \
        -lfftw3f \
        -lxml2 \
        -lpng \
        -mwin32 -mthreads -lpcreposix -lpcre -ljansson -lpthread
}

#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_ PROJECT_PATH=\"\\\"$$PWD\\\"\"
    LIBS += -lneurolabi_debug
    PRE_TARGETDEPS += $${NEUROLABI_DIR}/c/lib/libneurolabi_debug.a
} else {
    #DEFINES += _ADVANCED_
    LIBS += -lneurolabi
    PRE_TARGETDEPS += $${NEUROLABI_DIR}/c/lib/libneurolabi.a
}

#Self-contained libraries
unix {
    LIBS += -L$${EXTLIB_DIR}/xml/lib -L$${EXTLIB_DIR}/fftw3/lib \
        -L$${EXTLIB_DIR}/png/lib -L$${EXTLIB_DIR}/jansson/lib \
        -lfftw3 \
        -lfftw3f \
        -lxml2 \
        -lpng \
        -ljansson
}

exists($${EXTLIB_DIR}/hdf5/lib/libhdf5.a) {
    DEFINES += _ENABLE_HDF5_
    INCLUDEPATH += $${EXTLIB_DIR}/hdf5/include
    LIBS += -L$${EXTLIB_DIR}/hdf5/lib -lhdf5
}

#System libraries
unix {
    LIBS += -ldl -lz
}
