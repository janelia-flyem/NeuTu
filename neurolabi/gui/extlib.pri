DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext


#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    LIBS += -lneurolabi_debug
    PRE_TARGETDEPS += $${NEUROLABI_DIR}/c/lib/libneurolabi_debug.a
} else {
    #DEFINES += _ADVANCED_
    LIBS += -lneurolabi
    PRE_TARGETDEPS += $${NEUROLABI_DIR}/c/lib/libneurolabi.a
}

unix {
    INCLUDEPATH += \
        $${EXTLIB_DIR}/fftw3/include \
        $${EXTLIB_DIR}/png/include \
        $${EXTLIB_DIR}/jansson/include
}

unix:!macx {
    INCLUDEPATH += $${EXTLIB_DIR}/xml/include/libxml2
}

#System libraries
macx {
    INCLUDEPATH += /usr/include/libxml2
}

win32 {
    INCLUDEPATH += $${EXTLIB_DIR}/Mingw/64/include \
        $${EXTLIB_DIR}/Mingw/64/include/libxml2

    LIBS += -L$${EXTLIB_DIR}/Mingw/64/lib \
        -lfftw3 \
        -lfftw3f \
        -lxml2 \
        -lpng \
        -mwin32 -mthreads -lpcreposix -lpcre -ljansson -lpthread
}

#Self-contained libraries
unix {
    LIBS += -L$${EXTLIB_DIR}/fftw3/lib \
        -L$${EXTLIB_DIR}/png/lib -L$${EXTLIB_DIR}/jansson/lib \
        -lfftw3 \
        -lfftw3f \
        -lpng \
        -ljansson
}

unix:!macx {
    LIBS += -L$${EXTLIB_DIR}/xml/lib -lxml2
}

#System libraries
macx {
    LIBS += -lxml2
}

exists($${EXTLIB_DIR}/hdf5/lib/libhdf5.a) {
    DEFINES += _ENABLE_HDF5_
    INCLUDEPATH += $${EXTLIB_DIR}/hdf5/include
    LIBS += -L$${EXTLIB_DIR}/hdf5/lib -lhdf5 -lhdf5_hl
}

#System libraries
unix {
    LIBS += -ldl -lz
}
