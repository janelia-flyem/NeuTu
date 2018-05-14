ITK_VER = 4.13
ITK_INCLUDE_PATH = $${CONDA_ENV}/include/ITK-$${ITK_VER}
contains(DEFINES, _USE_ITK_) {
    system(echo ITK)
#    ITK_LIB_PATH = ../lib/ITK/lib
    INCLUDEPATH += $$ITK_INCLUDE_PATH
#    LIBS += -L$$ITK_LIB_PATH
#    DEFINES += _USE_ITK_
    HEADERS += itkimagedefs.h

    LIBS += -lITKCommon-$${ITK_VER} \
      -lITKVNLInstantiation-$${ITK_VER} \
      -litkvnl_algo-$${ITK_VER} \
      -litkvnl-$${ITK_VER} \
      -litkvcl-$${ITK_VER} \
      -litksys-$${ITK_VER} \
      -litkv3p_netlib-$${ITK_VER} \
      -lITKTransform-$${ITK_VER} \
      -lITKLabelMap-$${ITK_VER} \
      -lITKStatistics-$${ITK_VER} \
      -litkOptimizers-$${ITK_VER}
}
