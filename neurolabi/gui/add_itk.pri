exists(/usr/include/InsightToolkit) {
  ITK_INCLUDE_PATH = /usr/include/InsightToolkit
  ITK_LIB_PATH = /usr/lib/InsightToolkit
}

exists(/usr/local/include/ITK-4.2) {
  ITK_INCLUDE_PATH = /usr/local/include/ITK-4.2
    DEFINES += _USE_ITK_
    INCLUDEPATH += $$ITK_INCLUDE_PATH
    LIBS += -lITKCommon-4.2 \
      -litkv3p_lsqr-4.2 \
      -lITKVNLInstantiation-4.2 \
      -litkvnl_algo-4.2 \
      -litkvnl-4.2 \
      -litkvcl-4.2 \
      -litksys-4.2 \
      -litkv3p_netlib-4.2 \
      -lITKIOImageBase-4.2 -lITKIOTIFF-4.2 \
      -lITKLabelMap-4.2 -litktiff-4.2 -litkzlib-4.2 \
      -litkjpeg-4.2 -lITKStatistics-4.2 -lITKNetlibSlatec-4.2 \
      -litkOptimizers-4.2

    HEADERS += itkimagedefs.h
}

exists(/usr/local/include/ITK-4.3) {
  ITK_INCLUDE_PATH = /usr/local/include/ITK-4.3
    DEFINES += _USE_ITK_
    INCLUDEPATH += $$ITK_INCLUDE_PATH
    LIBS += -lITKCommon-4.3 \
      -litkv3p_lsqr-4.3 \
      -lITKVNLInstantiation-4.3 \
      -litkvnl_algo-4.3 \
      -litkvnl-4.3 \
      -litkvcl-4.3 \
      -litksys-4.3 \
      -litkv3p_netlib-4.3 \
      -lITKIOImageBase-4.3 -lITKIOTIFF-4.3 \
      -lITKLabelMap-4.3 -litktiff-4.3 -litkzlib-4.3 \
      -litkjpeg-4.3 -lITKStatistics-4.3 -lITKNetlibSlatec-4.3 \
      -litkOptimizers-4.3

    HEADERS += itkimagedefs.h
}

exists(/usr/local/include/ITK-4.4) {
  ITK_INCLUDE_PATH = /usr/local/include/ITK-4.4
    DEFINES += _USE_ITK_
    INCLUDEPATH += $$ITK_INCLUDE_PATH
    LIBS += -L/usr/local/lib -lITKCommon-4.4 \
      -litkv3p_lsqr-4.4 \
      -lITKVNLInstantiation-4.4 \
      -litkvnl_algo-4.4 \
      -litkvnl-4.4 \
      -litkvcl-4.4 \
      -litkdouble-conversion-4.4 -litksys-4.4 \
      -litkv3p_netlib-4.4 \
      -lITKIOImageBase-4.4 -lITKIOTIFF-4.4 \
      -lITKLabelMap-4.4 -litktiff-4.4 -litkzlib-4.4 \
      -litkjpeg-4.4 -lITKStatistics-4.4 -lITKNetlibSlatec-4.4 \
      -litkOptimizers-4.4

    HEADERS += itkimagedefs.h
}

exists($HOME/local/include/ITK-4.3) {
    ITK_INCLUDE_PATH = /groups/flyem/home/zhaot/local/include/ITK-4.3
    DEFINES += _USE_ITK_
    INCLUDEPATH += $$ITK_INCLUDE_PATH
    LIBS += -lITKCommon-4.3 \
        -litkv3p_lsqr-4.3 \
        -lITKVNLInstantiation-4.3 \
        -litkvnl_algo-4.3 \
        -litkvnl-4.3 \
        -litkvcl-4.3 \
        -litksys-4.3 \
        -litkv3p_netlib-4.3 \
        -lITKIOImageBase-4.3 -lITKIOTIFF-4.3 \
        -lITKLabelMap-4.3 -litktiff-4.3 -litkzlib-4.3 \
        -litkjpeg-4.3 -lITKStatistics-4.3 -litkNetlibSlatec-4.3 \
        -litkOptimizers-4.3

   HEADERS += itkimagedefs.h
}

exists(/usr/local/include/InsightToolkit) | exists(/usr/include/InsightToolkit) {
    DEFINES += _USE_ITK_
    INCLUDEPATH += $$ITK_INCLUDE_PATH \
      $$ITK_INCLUDE_PATH/BasicFilters \
      $$ITK_INCLUDE_PATH/Algorithms \
      $$ITK_INCLUDE_PATH/Common \
      $$ITK_INCLUDE_PATH/Numerics \
      $$ITK_INCLUDE_PATH/Utilities \
      $$ITK_INCLUDE_PATH/Utilities/vxl/core \
      $$ITK_INCLUDE_PATH/Utilities/vxl/vcl
    LIBS += -L$$ITK_LIB_PATH \
      -lITKCommon \
      -litkvnl_algo \
      -litkvnl \
      -litkvcl \
      -litksys \
      -litkv3p_netlib

    HEADERS += itkimagedefs.h
}

exists(/usr/local/include/opencv/cv.h) {
    DEFINES += _USE_OPENCV_
    INCLUDEPATH += /usr/local/include/opencv
    LIBS += -lopencv_core -lopencv_ml
}

exists(../lib/hdf5/lib/libhdf5.a) {
    DEFINES += _ENABLE_HDF5_
    INCLUDEPATH += ../lib/hdf5/include
    LIBS += -L../lib/hdf5/lib -lhdf5 -lz
}
