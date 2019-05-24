NEUROLABI_DIR = $${PWD}/..
EXTLIB_DIR = $${NEUROLABI_DIR}/lib
DEPENDPATH += . $${NEUROLABI_DIR}/c $${NEUROLABI_DIR}/c/include
INCLUDEPATH += $${NEUROLABI_DIR}/gui \
    $${NEUROLABI_DIR}/c \
    $${NEUROLABI_DIR}/c/include \
    $${EXTLIB_DIR}/genelib/src $${NEUROLABI_DIR}/gui/ext

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
    INCLUDEPATH += $${CONDA_ENV}/include
    LIBS += -L$${CONDA_ENV}/lib
    unix: QMAKE_RPATHDIR *= $${CONDA_ENV}/lib
    DEFINES += _ENABLE_LIBDVIDCPP_

#    LIBS += $${CONDA_ENV}/lib/libhdf5.la $${CONDA_ENV}/lib/libhdf5_hl.la
#    DEFINES += _ENABLE_HDF5_
}

exists($${CONDA_ENV}) {
#  LIBXML_DIR = $${CONDA_ENV}
  LIBJANSSON_DIR = $${CONDA_ENV}
  LIBFFTW_DIR = $${CONDA_ENV}
} else {
#  LIBXML_DIR = $${EXTLIB_DIR}/xml
  LIBJANSSON_DIR = $${EXTLIB_DIR}/jansson
  LIBFFTW_DIR = $${EXTLIB_DIR}/fftw3
}

unix {
  #neurolabi
  LIBS += -L$${NEUROLABI_DIR}/c/lib


  equals(SANITIZE_BUILD, "address") {
    LIBS += -lneurolabi_sanitize
  } else {
    CONFIG(debug, debug|release) {
      LIBS += -lneurolabi_debug
    } else {
      LIBS += -lneurolabi
    }
  }

  INCLUDEPATH += $${LIBFFTW_DIR}/include \
#        $${EXTLIB_DIR}/png/include \
        $${LIBJANSSON_DIR}/include
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
    LIBS += -L$${LIBFFTW_DIR}/lib \
        -L$${LIBJANSSON_DIR}/lib \
        -lfftw3 \
        -lfftw3f \
        -ljansson
}

#exists($${EXTLIB_DIR}/hdf5/lib/libhdf5.a) {
#    message("hdf5 enabled")
#    DEFINES += _ENABLE_HDF5_
#    INCLUDEPATH += $${EXTLIB_DIR}/hdf5/include
##    LIBS += -L$${EXTLIB_DIR}/hdf5/lib -lhdf5 -lhdf5_hl
#}

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

contains(DEFINES, _ENABLE_SURFRECON_) {
  LIBS+=-lCGAL -lCGAL_Core -lgmp

  #-lsurfrecon
#  QMAKE_CXXFLAGS+=-fext-numeric-literals
}

VTK_VER = 8.2

#
exists($${CONDA_ENV}) {
#  VTK_VER = 7.1
  INCLUDEPATH += $${CONDA_ENV}/include $${CONDA_ENV}/include/draco/src
  LIBS += -L$${CONDA_ENV}/lib -lglbinding -lassimp -ldracoenc -ldracodec -ldraco -larchive -lrdkafka++
  INCLUDEPATH += $${CONDA_ENV}/include/vtk-$${VTK_VER}
} else {
  INCLUDEPATH += $$PWD/ext/glbinding/include $$PWD/ext/assimp/include $$PWD/ext/draco/include/draco/src
  LIBS += -L$$PWD/ext/glbinding/lib -lglbinding -L$$PWD/ext/assimp/lib -lassimp -L$$PWD/ext/draco/lib -ldracoenc -ldracodec -ldraco
  INCLUDEPATH += vtk-$${VTK_VER}
}

LIBS += -lvtkFiltersGeometry-$${VTK_VER} -lvtkCommonCore-$${VTK_VER} \
    -lvtksys-$${VTK_VER} -lvtkCommonDataModel-$${VTK_VER} \
    -lvtkCommonMath-$${VTK_VER} -lvtkCommonMisc-$${VTK_VER} \
    -lvtkCommonSystem-$${VTK_VER} -lvtkCommonTransforms-$${VTK_VER} \
    -lvtkCommonExecutionModel-$${VTK_VER} -lvtkFiltersCore-$${VTK_VER} \
    -lvtkFiltersSources-$${VTK_VER} -lvtkCommonComputationalGeometry-$${VTK_VER} \
    -lvtkFiltersGeneral-$${VTK_VER}

win32 {
  LIBS += -lopengl32 -lglu32
}
macx {
  LIBS += -framework AGL -framework OpenGL
}
#unix:!macx {
#  LIBS += -lGL -lGLU
#}

CONFIG(static_gtest) { # gtest from ext folder
  include($$PWD/ext/gtest.pri)
}

include(ext/QsLog/QsLog.pri)
include(ext/libqxt.pri)

HEADERS += ext/http/HTTPRequest.hpp
