NEUROLABI_DIR = $${PWD}/..
EXTLIB_DIR = $${NEUROLABI_DIR}/lib
DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext


unix {
#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    contains(CONFIG, sanitize) {
      LIBS += -lneurolabi_sanitize
    } else {
      LIBS += -lneurolabi_debug
    }
} else {
    #DEFINES += _ADVANCED_
    LIBS += -lneurolabi
}

    INCLUDEPATH += $${EXTLIB_DIR}/xml/include/libxml2 \
        $${EXTLIB_DIR}/fftw3/include \
#        $${EXTLIB_DIR}/png/include \
        $${EXTLIB_DIR}/jansson/include
}

win32 {
#neurolabi
LIBS += -L$${NEUROLABI_DIR}/c/lib
CONFIG(debug, debug|release) {
    LIBS += neurolabi_debug.lib
} else {
    #DEFINES += _ADVANCED_
    LIBS += neurolabi.lib
}

    DEFINES += LIBXML_STATIC PCRE2_STATIC

    INCLUDEPATH += $${EXTLIB_DIR}/msvc/zlib/include \
        $${EXTLIB_DIR}/msvc/libxml2/include/libxml2 \
        $${EXTLIB_DIR}/msvc/jansson/include \
        $${EXTLIB_DIR}/msvc/pcre/include \
        $${EXTLIB_DIR}/msvc/fftw

    LIBS += -L$${EXTLIB_DIR}/msvc/zlib/lib zlibstatic.lib \
        -L$${EXTLIB_DIR}/msvc/jansson/lib jansson.lib \
        -L$${EXTLIB_DIR}/msvc/libxml2/lib libxml2_a.lib \
        -L$${EXTLIB_DIR}/msvc/pcre/lib pcre2posix.lib pcre2-8.lib \
        -L$${EXTLIB_DIR}/msvc/fftw libfftw3f-3.lib libfftw3-3.lib

    LIBS += $${QMAKE_LIBS_OPENGL} $${QMAKE_LIBS_NETWORK} $${QMAKE_LIBS_GUI} $${QMAKE_LIBS_CORE}
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
    unix: QMAKE_RPATHDIR *= $${CONDA_ENV}/lib
    DEFINES += _ENABLE_LIBDVIDCPP_
}

message("rpath")
message($$QMAKE_RPATHDIR)

contains(DEFINES, _ENABLE_LIBDVIDCPP_) {
    LIBS *= -ldvidcpp -lboost_system #-lboost_thread -ljsoncpp -llz4 -lcurl -lpng -ljpeg
    contains(DEFINES, _ENABLE_LOWTIS_) {
        LIBS *= -llowtis
#        CONFIG(debug, debug|release) {
#            LIBS *= -llowtis-g
#        } else {
#            LIBS *= -llowtis
#        }
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
