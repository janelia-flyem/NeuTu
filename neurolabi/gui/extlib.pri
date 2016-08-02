EXTLIB_DIR = $${NEUROLABI_DIR}/lib
DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext


#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    LIBS += -lneurolabi_debug
} else {
    #DEFINES += _ADVANCED_
    LIBS += -lneurolabi
}

unix {
    INCLUDEPATH += $${EXTLIB_DIR}/xml/include/libxml2 \
        $${EXTLIB_DIR}/fftw3/include \
#        $${EXTLIB_DIR}/png/include \
        $${EXTLIB_DIR}/jansson/include
}

win32 {
    DEFINES += LIBXML_STATIC
    INCLUDEPATH += $${EXTLIB_DIR}/Mingw/64/include \
        $${EXTLIB_DIR}/Mingw/64/include/libxml2

    LIBS += -L$${EXTLIB_DIR}/Mingw/64/lib \
        -lfftw3 \
        -lfftw3f \
        -lxml2 \
#        -lpng \
        -mwin32 -mthreads -lpcre2posix -lpcre2-8 -ljansson -lpthread
}

#Self-contained libraries
unix {
    LIBS += -L$${EXTLIB_DIR}/xml/lib -L$${EXTLIB_DIR}/fftw3/lib \
        -L$${EXTLIB_DIR}/jansson/lib \
        -lfftw3 \
        -lfftw3f \
        -lxml2 \
        -ljansson
}

exists($${EXTLIB_DIR}/hdf5/lib/libhdf5.a) {
    DEFINES += _ENABLE_HDF5_
    INCLUDEPATH += $${EXTLIB_DIR}/hdf5/include
    LIBS += -L$${EXTLIB_DIR}/hdf5/lib -lhdf5 -lhdf5_hl
}

#System libraries
unix {
#tmp fix
    #LIBS += -L/usr/lib64/ssl
    LIBS += -ldl -lz
}

CONFIG(debug, debug|release) {
    exists($${EXTLIB_DIR}/opencv) {
        message(opencv found)
        DEFINES += _USE_OPENCV_
        INCLUDEPATH += $${EXTLIB_DIR}/opencv/include $${EXTLIB_DIR}/opencv/include/opencv
        LIBS += -L$${EXTLIB_DIR}/opencv/lib -lopencv_core -lopencv_ml
    }
}

contains(TEMPLATE, app) {
exists($$DVIDCPP_PATH) {
    DEFINES += _ENABLE_LIBDVIDCPP_
    INCLUDEPATH += $$DVIDCPP_PATH/include
    LIBS += -L$$DVIDCPP_PATH/lib
    DEFINES += _LIBDVIDCPP_OLD_
} else:exists($${BUILDEM_DIR}) {
    INCLUDEPATH +=  $${BUILDEM_DIR}/include
    LIBS += -L$${BUILDEM_DIR}/lib -L$${BUILDEM_DIR}/lib64
    DEFINES += _ENABLE_LIBDVIDCPP_
} else:exists($${CONDA_ENV}) {
    INCLUDEPATH +=  $${CONDA_ENV}/include
    LIBS += -L$${CONDA_ENV}/lib
    DEFINES += _ENABLE_LIBDVIDCPP_
}

contains(DEFINES, _ENABLE_LIBDVIDCPP_) {
    LIBS *= -ldvidcpp -ljsoncpp -llz4 -lcurl -lpng -ljpeg -lboost_system -lboost_thread
    contains(DEFINES, _ENABLE_LOWTIS_) {
        CONFIG(debug, debug|release) {
            LIBS *= -llowtis-g
        } else {
            LIBS *= -llowtis
        }
    }

    !contains(DEFINES, _LIBDVIDCPP_OLD_) {
        LIBS *= -lssl -lcrypto
    }
} else:exists($${EXTLIB_DIR}/png/lib) {
    LIBS += -L$${EXTLIB_DIR}/png/lib -lpng
}
}


message($$DEFINES)
message($$LIBS)

#BUILDEM_DIR = /opt/Downloads/buildem
#exists($${BUILDEM_DIR}/lib/libdvidcpp2.a) {
#    DEFINES += _ENABLE_LIBDVID_
#    INCLUDEPATH +=  $${BUILDEM_DIR}/include $${BUILDEM_DIR}/include/libdvid
#    LIBS += -L$${BUILDEM_DIR}/lib -L$${BUILDEM_DIR}/lib64 -ldvidcpp \
#        -ljsoncpp -lcppnetlib-uri \
#        -lcppnetlib-client-connections -lcppnetlib-server-parsers  \
#        -lboost_system -lboost_thread -lssl -lcrypto
#}
