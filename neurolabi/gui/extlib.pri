DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${EXTLIB_DIR}/xml/include/libxml2 \
    $${EXTLIB_DIR}/fftw3/include \
    $${EXTLIB_DIR}/png/include \
    $${EXTLIB_DIR}/jansson/include \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext

#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_ PROJECT_PATH=\"\\\"$$PWD\\\"\"
    LIBS += -lneurolabi
} else {
    DEFINES += _ADVANCED_
    LIBS += -lneurolabi
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

#System libraries
unix {
    LIBS += -ldl -lz
}
